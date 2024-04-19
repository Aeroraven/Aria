//With Reference: https://threejs.org/examples/?q=unreal#webgl_postprocessing_unreal_bloom

struct VSOutput
{
    [[vk::location(0)]] float2 texCoord : TEXCOORD0;
};

Texture2D texIn : register(t0, space0);
SamplerState sampIn : register(s0, space0);
Texture2D texMip[5] : register(t0, space1);
SamplerState sampMip[5] : register(s0, space1);

static const float bloomStrength = 0.4;
static const float bloomRadius = 0.1;
static const float toneExposure = 1.5;

float lerpBloomFactor(const in float factor)
{
    float mirrorFactor = 1.2 - factor;
    return lerp(factor, mirrorFactor, bloomRadius);
}

float4 bloom(float2 texCoord)
{
    float4 accum = 0;
    float bloomFactors[5] = { 1.0, 0.8, 0.6, 0.4, 0.2 };
    for(int i = 0; i < 5; i++)
    {
        accum += texMip[i].Sample(sampMip[i], texCoord) * lerpBloomFactor(bloomFactors[i]) * bloomStrength;
    }
    return accum;

}
float4 toneMapping(float4 color)
{
    return float4(1, 1, 1, 1) - exp(-color * toneExposure);
}
float4 inverseGamma(float4 color)
{
    return pow(color, 2.2);
}
float4 gamma(float4 color)
{
    return pow(color, 1.0 / 2.2);
}
float4 main(VSOutput vsOut) : SV_Target0
{
    float4 baseColor = inverseGamma(texIn.Sample(sampIn, vsOut.texCoord));
    return gamma(toneMapping(bloom(vsOut.texCoord) + baseColor));
}