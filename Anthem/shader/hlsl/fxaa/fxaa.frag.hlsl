struct VSOutput
{
    [[vk::location(0)]] float2 texCoord : TEXCOORD0;
};

Texture2D texIn[3] : register(t0, space0);
SamplerState sampIn[3] : register(s0, space0);

static const float LUMIN_MIN_THRESH = 0.0712;
static const float LUMIN_SCALE_MIN_THRESH = 0.033;
static const float MAX_SEARCH_STEPS = 20.0;

float lumin(float4 color)
{
    return color.r * 0.213 + color.g * 0.715 + color.b * 0.072;
}

float luminTex(float2 texc)
{
    return lumin(texIn[0].Sample(sampIn[0], texc));
}

float4 main(VSOutput vsOut) : SV_Target0
{
    
    uint texHeight, texWidth, texLods;
    texIn[0].GetDimensions(0, texWidth, texHeight, texLods);
    float dx = 1.0 / texWidth, dy = 1.0 / texHeight;
    float2 cpos = vsOut.texCoord;
    
    float4 cC = texIn[0].Sample(sampIn[0], cpos);
    float4 cL = texIn[0].Sample(sampIn[0], cpos + float2(-dx, 0));
    float4 cR = texIn[0].Sample(sampIn[0], cpos + float2(+dx, 0));
    float4 cT = texIn[0].Sample(sampIn[0], cpos + float2(0, -dy));
    float4 cB = texIn[0].Sample(sampIn[0], cpos + float2(0, +dy));
    
    float lC = lumin(cC),lL = lumin(cL),lR = lumin(cR), lT = lumin(cT),lB = lumin(cB);
    
    float lMax = max(lC, max(lL, max(lR, max(lT, lB))));
    float lMin = min(lC, min(lL, min(lR, min(lT, lB))));
    float contrast = lMax - lMin;

    if (contrast > min(LUMIN_MIN_THRESH, lMax * LUMIN_SCALE_MIN_THRESH)) //
    {
        float4 cLT = texIn[0].Sample(sampIn[0], cpos + float2(-dx, -dy));
        float4 cRT = texIn[0].Sample(sampIn[0], cpos + float2(dx, -dy));
        float4 cLB = texIn[0].Sample(sampIn[0], cpos + float2(-dx, dy));
        float4 cRB = texIn[0].Sample(sampIn[0], cpos + float2(dx, dy));
        
        float lLT = lumin(cLT), lRT = lumin(cRT), lLB = lumin(cLB), lRB = lumin(cRB);
        float filter = ((lL + lR + lT + lB) * 2 + (lRT + lLT + lLB + lRB) * 1) / 12;
        filter = saturate(abs(filter - lC) / contrast);
        float pxBlendFactor = pow(smoothstep(0, 1, filter), 2);
        
        float verticalDelta = abs(lT + lB - 2 * lC) * 2 + abs(lLT + lLB - 2 * lL) + abs(lRT + lRB - 2 * lR);
        float horizontalDelta = abs(lL + lR - 2 * lC) * 2 + abs(lLT + lRT - 2 * lT) + abs(lLB + lRB - 2 * lB);
        float2 blendDirection = float2(0, 0);
        float2 edgeSearchDir = float2(0, dy);
        if (verticalDelta > horizontalDelta){
            edgeSearchDir = float2(dx, 0);
            if (abs(lT - lC) > abs(lB - lC))
                blendDirection = float2(0, -1);
            else
                blendDirection = float2(0, 1);
        }
        else{
            if(abs(lL-lC)>abs(lR-lC))
                blendDirection = float2(-1, 0);
            else 
                blendDirection = float2(1, 0);
        }
        
        float2 baseTexW = vsOut.texCoord + 0.5 * blendDirection * float2(dx, dy);
        float baseLumin = (luminTex(baseTexW)) / 2;
        float stopCriteria = baseLumin * 0.25;
        
        float posSteps = 0, posDelta = 0;
        float negSteps = 0, negDelta = 0;

        for (; posSteps <= MAX_SEARCH_STEPS; posSteps += 1.0){
            posDelta = (luminTex(baseTexW + posSteps * edgeSearchDir))/2 - baseLumin;
            if (abs(posDelta) > stopCriteria)
                break;
        }
        for (; negSteps <= MAX_SEARCH_STEPS; negSteps += 1.0){
            negDelta = (luminTex(baseTexW - negSteps * edgeSearchDir)) / 2 - baseLumin;
            if(abs(negDelta)>stopCriteria)
                break;
        }
        
        
        float edgeBlendFactor = 0;
        if (posSteps < negSteps){
            if (sign(posDelta) != sign(lC - baseLumin)){
                edgeBlendFactor = 0.5 - posSteps / (posSteps + negSteps);
            }
        }
        else {
            if (sign(negDelta) != sign(lC - baseLumin)){
                edgeBlendFactor = 0.5 - negSteps / (posSteps + negSteps);
            }
        }
        
        float finalFactor = max(edgeBlendFactor, pxBlendFactor);
        return lerp(texIn[0].Sample(sampIn[0], vsOut.texCoord), texIn[0].Sample(sampIn[0], vsOut.texCoord + blendDirection * float2(dx, dy)), finalFactor);

    }
    return cC;
}