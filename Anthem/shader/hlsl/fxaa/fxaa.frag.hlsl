struct VSOutput
{
    [[vk::location(0)]] float2 texCoord : TEXCOORD0;
};

Texture2D texIn : register(t0, space0);
SamplerState sampIn : register(s0, space0);

static const float LUMIN_MIN_THRESH = 0.0112;
static const float LUMIN_SCALE_MIN_THRESH = 0.033;
static const float MAX_SEARCH_STEPS = 20.0;

float lumin(float4 color)
{
    return color.r * 0.213 + color.g * 0.715 + color.b * 0.072;
}

float luminTex(float2 texc)
{
    return lumin(texIn.Sample(sampIn, texc));
}

float4 sobel(float2 texc)
{
    float texH, texW, texLods;
    texIn.GetDimensions(0, texW, texH, texLods);
    float dx = 1.0 / texW, dy = 1.0 / texH;
    float4 cC = texIn.Sample(sampIn, texc);
    float4 cL = texIn.Sample(sampIn, texc + float2(-dx, 0));
    float4 cR = texIn.Sample(sampIn, texc + float2(+dx, 0));
    float4 cT = texIn.Sample(sampIn, texc + float2(0, -dy));
    float4 cB = texIn.Sample(sampIn, texc + float2(0, +dy));
    float4 cLT = texIn.Sample(sampIn, texc + float2(-dx, -dy));
    float4 cRT = texIn.Sample(sampIn, texc + float2(dx, -dy));
    float4 cLB = texIn.Sample(sampIn, texc + float2(-dx, dy));
    float4 cRB = texIn.Sample(sampIn, texc + float2(dx, dy));
    float lC = lumin(cC), lL = lumin(cL), lR = lumin(cR), lT = lumin(cT), lB = lumin(cB);
    float lLT = lumin(cLT), lRT = lumin(cRT), lLB = lumin(cLB), lRB = lumin(cRB);
    float4 horizEdge = cL + cL + cL + cR + cR + cR - cT - cT - cT - cB - cB - cB;
    float4 vertEdge = cT + cT + cT + cB + cB + cB - cL - cL - cL - cR - cR - cR;
    return sqrt(horizEdge * horizEdge + vertEdge * vertEdge);
}

float4 main(VSOutput vsOut) : SV_Target0
{
    //return texIn.Sample(sampIn, vsOut.texCoord);
    //return texIn.SampleLevel(sampIn, vsOut.texCoord, 4);
    //return sobel(vsOut.texCoord);
    
    uint texHeight, texWidth, texLods;
    texIn.GetDimensions(0, texWidth, texHeight, texLods);
    float dx = 1.0 / texWidth, dy = 1.0 / texHeight;
    float2 cpos = vsOut.texCoord;
    
    float4 cC = texIn.Sample(sampIn, cpos);
    float4 cL = texIn.Sample(sampIn, cpos + float2(-dx, 0));
    float4 cR = texIn.Sample(sampIn, cpos + float2(+dx, 0));
    float4 cT = texIn.Sample(sampIn, cpos + float2(0, -dy));
    float4 cB = texIn.Sample(sampIn, cpos + float2(0, +dy));
    
    float lC = lumin(cC),lL = lumin(cL),lR = lumin(cR), lT = lumin(cT),lB = lumin(cB);
    
    float lMax = max(lC, max(lL, max(lR, max(lT, lB))));
    float lMin = min(lC, min(lL, min(lR, min(lT, lB))));
    float contrast = lMax - lMin;
    
    //return float4(0, abs(lC - lMin),0,0);
    
    if (contrast > min(LUMIN_MIN_THRESH, lMax * LUMIN_SCALE_MIN_THRESH)) //
    {
        float4 cLT = texIn.Sample(sampIn, cpos + float2(-dx, -dy));
        float4 cRT = texIn.Sample(sampIn, cpos + float2(dx, -dy));
        float4 cLB = texIn.Sample(sampIn, cpos + float2(-dx, dy));
        float4 cRB = texIn.Sample(sampIn, cpos + float2(dx, dy));
        
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
        
        float2 baseTexC = vsOut.texCoord;
        float2 baseTexT = vsOut.texCoord + blendDirection * float2(dx, dy);
        float baseLumin = (luminTex(baseTexC) + luminTex(baseTexT)) / 2;
        float stopCriteria = baseLumin * 0.25;
        
        float posSteps = 0, posDelta = 0;
        float negSteps = 0, negDelta = 0;

        for (; posSteps <= MAX_SEARCH_STEPS; posSteps += 1.0){
            posDelta = (luminTex(baseTexC + posSteps * edgeSearchDir) + luminTex(baseTexT + posSteps * edgeSearchDir))/2 - baseLumin;
            if (abs(posDelta) > stopCriteria)
                break;
        }
        for (; negSteps <= MAX_SEARCH_STEPS; negSteps += 1.0){
            negDelta = (luminTex(baseTexC - negSteps * edgeSearchDir) + luminTex(baseTexT - negSteps * edgeSearchDir)) / 2 - baseLumin;
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
        return lerp(texIn.Sample(sampIn, vsOut.texCoord), texIn.Sample(sampIn, vsOut.texCoord + blendDirection * float2(dx, dy)), finalFactor);

    }
    //return float4(0, 0, 0, 0);
    return texIn.Sample(sampIn, vsOut.texCoord);
}