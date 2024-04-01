#include "volfog.general.hlsl"

struct LightAttrs
{
    int4 volSize;
    float4 lightAttrs; //Density & Anisotropy
    float4 lightColor; //Color & Ambient
    float4 lightDir;
    float4 farNear;
    float4 camPos;
    float4x4 inverseVp;
};

struct LightMVP
{
    float4x4 proj;
    float4x4 view;
    float4x4 model;
};

ConstantBuffer<LightAttrs> attr : register(b0, space0);
RWTexture3D<float4> scatData : register(u0, space1);
Texture2D texShadow : register(t0, space2);
SamplerState sampShadow : register(s0, space2);
ConstantBuffer<LightMVP> camLight : register(b0, space3);

float isShadowed(float3 wPos)
{
    //TODO: Filtering
    float4 lightNdc = mul(camLight.proj, mul(camLight.view, mul(camLight.model, float4(wPos, 1.0))));
    lightNdc = lightNdc / lightNdc.w;
    float refDepth = texShadow.SampleLevel(sampShadow, (lightNdc.xy * 0.5 + 0.5), 0).r;
    float curDepth = lightNdc.z;
    if ((curDepth - refDepth) > 1e-3)
        return 1.0;
    return 0.0;
}

float phaseFunction(float3 wIn, float3 wOut, float aniso)
{
    float cAngle = dot(normalize(wIn), normalize(wOut));
    float g = aniso, g2 = pow(aniso, 2);
    float numo = 1 - g2, deno = pow(1 + g2 + 2 * g * cAngle, 3.0 / 2.0) * 4 * 3.14159;
    return numo / deno;
}
float phaseFunctionIL(float3 wIn, float3 wOut, float aniso)
{
    return 1.0 / 4.0 / 3.14159;
}


[numthreads(8, 8, 8)]
void main(uint3 invId : SV_DispatchThreadID)
{
    float3 worldPos = invId2WorldPos(invId, attr.volSize.xyz, attr.inverseVp, attr.farNear.x, attr.farNear.y).xyz;
    float3 rayDir = normalize(worldPos - attr.camPos.xyz);
    
    float3 light = float3(0, 0, 0);
    light += attr.lightColor.a;
    
    float visibility = 1.0 - isShadowed(worldPos);
    light += visibility * attr.lightColor.xyz * phaseFunctionIL(rayDir, -attr.lightDir.xyz, attr.lightAttrs.g);
    
    scatData[invId] = float4(light, attr.lightAttrs.r);
}