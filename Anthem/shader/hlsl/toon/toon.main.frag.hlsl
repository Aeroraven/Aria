struct VSOutput
{
    [[vk::location(0)]] float4 texUv: TEXCOORD0;
    [[vk::location(1)]] float4 normal: NORMAL0;
    [[vk::location(2)]] float4 texIndices : TEXCOORD1;
};
Texture2D texPbrBase[4] : register(t0, space1);
SamplerState sampPbrBase[4] : register(s0, space1);

static const float4 LIGHT_DIR = float4(normalize(float3(-2, 0, 8)), 0);
static const float TOON_SHARPNESS = 150.0f;

float4 getBaseColor(uint baseId, float2 uv)
{
    return texPbrBase[baseId].Sample(sampPbrBase[baseId], uv).rgba;
}
float4 gamma(float4 color)
{
    return pow(color, 1.0 / 2.2);
}
float4 inverseGamma(float4 color)
{
    return pow(color, 2.2);
}
float sigmodShift(float x, float shift, float sharpness)
{
    return 1.0 / (1.0 + exp(-sharpness * (x - shift)));
}
float intensityRamp(float diffuse)
{
    float th2 = 0.5;
    float ramp2 = sigmodShift(diffuse, th2, TOON_SHARPNESS);
    float ramp1 = 1 - ramp2;
    float ramp2w = 1.0;
    float ramp1w = th2;
    return ramp1 * ramp1w + ramp2 * ramp2w;
}

float4 main(VSOutput vsOut) : SV_Target
{
    float3 n = normalize(vsOut.normal.xyz);
    float3 l = normalize(LIGHT_DIR.xyz);
    float diff = max(0, dot(n, -l));
    float4 baseColor = float4(0.9, 0.9, 0.9, 1);//inverseGamma(getBaseColor(uint(vsOut.texIndices.r), vsOut.texUv.xy));
    float intensity = intensityRamp(diff);
    
    return gamma(baseColor * intensity);
    
    //return float4(0.8, 0.8, 0.8, 1);
    //return 
}
