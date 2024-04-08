#include "sine.general.hlsl"

// Lighting Model Reference:
// https://github.com/GarrettGunnell/Water

struct VSOutput
{
    [[vk::location(0)]] float4 pos : POSITION0;
};


ConstantBuffer<Constants> data : register(b0, space0);
TextureCube texSkybox : register(t0, space1);
SamplerState sampSkybox : register(s0, space1);


static const float SPECULAR_HIGHLIGHT = 64.0;
static const float4 DIFFUSE_REFLECTANCE = float4(0.01, 0.3, 0.8, 0);
static const float4 SPECULAR_REFLECTANCE = float4(1, 1, 1, 0);
static const float4 LIGHT_COLOR = float4(1, 1, 1, 0);
static const float4 SUN_COLOR = float4(3.01, 1.94, 0.87, 0);
static const float FRESNEL_SHININESS = 5.0;
static const float FRESNEL_BIAS = 0.02037;
static const float4 FRESNEL_COLOR = float4(1, 1, 1, 0);
static const float DIFFUSE_STRENGTH = 0.1;

float4 toSrgb(float4 color)
{
    return pow(color, float4(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
}

float3 toLinear(float3 color)
{
    return pow(color, float3( 2.2, 2.2,  2.2));
}
float4 main(VSOutput vsOut) : SV_Target0
{
    float4 baseColor = float4(0.1, 0.7, 1, 1);
    
    float2 p = vsOut.pos.xz;

    float3 dxyz = calcWave(p, data.timing.x, data.wave, data.freqAmpl.y, data.freqAmpl.x, data.freqAmpl.w, data.freqAmpl.z);
    float3 pos = float3(p.x, dxyz.y, p.y);
    
    float3 dx = normalize(float3(1, dxyz.x, 0));
    float3 dz = normalize(float3(0, dxyz.z, 1));
    float3 n = -cross(dx, dz);
    float dv = (dot(n, float3(0, 1, 0)));

    // Fresnel
    float3 Li = normalize(data.camPos.xyz - pos);
    float3 Lo = normalize(-data.sunDir.xyz);
    
    // Schlick Fresnel
    float sfBase = 1 - dot(Li, n);
    float sfExp = pow(sfBase, FRESNEL_SHININESS);
    float R = sfExp + FRESNEL_BIAS * (1.0f - sfExp);

    float3 refl = reflect(-Li, n);
    float3 skyColor = toLinear(texSkybox.Sample(sampSkybox, refl.xyz).rgb);
    float3 sun = SUN_COLOR.xyz * pow(max(0.0f, dot(refl, Lo)), 500.0f);

    float3 fresnel = skyColor * R;
    fresnel += sun * R;
    float4 fresnelF = float4(fresnel, 1.0f);
    
    //Specular
    float3 h = normalize(Li + Lo);
    float ds = pow(max(0, dot(h, n)), SPECULAR_HIGHLIGHT) * max(0, dot(n, Lo));
    
    float4 specular = LIGHT_COLOR * SPECULAR_REFLECTANCE * ds;
    float spBase = 1 - dot(Li, h);
    float spExp = pow(spBase, 5);
    specular *= FRESNEL_BIAS + (1 - FRESNEL_BIAS) * spExp;
    
    //Diffuse
    float dd = max(0, dot(n, Lo)) / 3.1415926;
    float4 diffuse = dd * DIFFUSE_REFLECTANCE * LIGHT_COLOR * DIFFUSE_STRENGTH;
    
    //Ambient
    float ambient = 0.01;
    
    float4 color = fresnelF + specular + diffuse + ambient;
    color = toSrgb(color);

    return float4(color);
}