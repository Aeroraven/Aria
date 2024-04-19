struct VSOutput
{
    [[vk::location(0)]] float4 texUv: TEXCOORD0;
    [[vk::location(1)]] float4 normal: NORMAL0;
    [[vk::location(2)]] float4 texIndices : TEXCOORD1;
};
Texture2D texPbrBase[4] : register(t0, space1);
SamplerState sampPbrBase[4] : register(s0, space1);

static const float4 LIGHT_DIR = float4(normalize(float3(-1, 0, 2)), 0);
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

float4 fresnelRimLight(float4 f0, float3 n, float3 v, float rimWid, float rimSoftness)
{
    float ndotv = dot(n, -v);
    float rimFactor = ndotv < 0 ? 0 : 1 - ndotv;
    float4 ret = f0 + (1.0f - f0) * pow(rimFactor, 5.0);
    ret = smoothstep(float4(rimWid, rimWid, rimWid, rimWid), float4(1, 1, 1, 1), ret);
    ret = smoothstep(float4(0, 0, 0, 0), float4(rimSoftness, rimSoftness, rimSoftness, rimSoftness), ret);
    
    return ret;
}

float4 main(VSOutput vsOut) : SV_Target
{
    //return float4(1, 1, 1, 1);
    float3 n = normalize(vsOut.normal.xyz);
    float3 l = normalize(LIGHT_DIR.xyz);
    float3 v = normalize(float3(0, 0, 1));
    float diff = max(0, dot(n, -l));
    float4 baseColor = inverseGamma(getBaseColor(uint(vsOut.texIndices.r), vsOut.texUv.xy));
    float intensity = intensityRamp(diff);
    float4 rim = fresnelRimLight(float4(0.02, 0.02, 0.02, 1), n, v, -0.1, 0.7) * diff;
    
    float alpha = rim.r;
    return float4(gamma(baseColor * (intensity + rim)).xyz, alpha);
    
    //return float4(0.8, 0.8, 0.8, 1);
    //return 
}
