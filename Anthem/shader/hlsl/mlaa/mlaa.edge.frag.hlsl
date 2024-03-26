struct VSOutput{
    [[vk::location(0)]] float2 texCoord : TEXCOORD0;
};

Texture2D texTarget : register(t0, space0);
SamplerState sampTarget : register(s0, space0);

float lumin(float4 color)
{
    return color.r * 0.213 + color.g * 0.715 + color.b * 0.072;
}

float luminTex(float2 loc)
{
    return lumin(texTarget.Sample(sampTarget, loc));
}

float4 main(VSOutput vsOut) : SV_TARGET0
{

    float texW, texH, texL;
    texTarget.GetDimensions(0, texW, texH, texL);
    float dx = 1 / texW, dy = 1 / texH;
    
    float2 texc = vsOut.texCoord;
    float lC = luminTex(texc);
    float lL = luminTex(texc + float2(-dx, 0));
    float lR = luminTex(texc + float2(dx, 0));
    float lT = luminTex(texc + float2(0, -dy));
    float lB = luminTex(texc + float2(0, dy));
    
    float thSingle = 0.05;
    float4 thresh = float4(thSingle, thSingle, thSingle, thSingle);
    float4 delta = abs(float4(lL, lR, lT, lB) - lC);
    float4 threshDelta = step(thresh, delta);
    if (dot(threshDelta, float4(1, 1, 1, 1)) < 1e-4){
        return float4(0, 0, 0, 0);
    }
    return threshDelta;
    
}