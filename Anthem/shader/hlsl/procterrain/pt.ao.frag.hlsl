#include "pt.common.hlsl"

struct VSOutput
{
    [[vk::location(0)]] float4 texUv : TEXCOORD0;
};
struct AOParams
{
    float4 vec[64];
};
struct PSOutput
{
    float4 ao : SV_Target0;
};

Texture2D texNormal : register(t0, space0);
SamplerState sampNormal : register(s0, space0);
Texture2D texPosition : register(t0, space1);
SamplerState sampPosition : register(s0, space1);

ConstantBuffer<AOParams> attr:register(b0, space2);
ConstantBuffer<Camera> cam : register(b0, space3);
static const float PI2 = 6.28318530718;

float random(float2 seeds)
{
    return frac(sin(dot(seeds, float2(12.9898, 4.1414))) * 43758.5453);
}

float3 randomUnitVector(float2 co)
{
    float2 seed1 = co.xy;
    float2 seed2 = co.yx;
    float3 randVec = float3(random(seed1), random(seed2 + 1.0), random(seed1 + seed2 + 2.0));
    return normalize(randVec * 2.0 - 1.0);
}

PSOutput main(VSOutput vsOut)
{
    PSOutput psOut;
    float4 posr = texPosition.Sample(sampPosition, vsOut.texUv.xy).xyzw;
    if (posr.a < 0.5)
    {
        psOut.ao = float4(0, 0, 0, 0);
        return psOut;
    }
    else if (posr.a > 1.5)
    {
        psOut.ao = float4(1, 0, 0, 0);
        return psOut;
    }
        float3 pos = posr.xyz;
    float3 normal = texNormal.Sample(sampNormal, vsOut.texUv.xy).xyz;
    
    // Generate TBM Matrix
    float3 tangent = randomUnitVector(vsOut.texUv.xy); //texTangent.Sample(sampTangent, vsOut.texUv.xy).xyz;
    tangent = normalize(tangent - dot(tangent, normal) * normal);
    float3 bitangent = cross(normal, tangent);
    float3x3 tbn = transpose(float3x3(tangent, bitangent, normal));
    float4x4 mvp = mul(cam.proj, mul(cam.view, cam.model));
    
    // Calculate AO
    float occls = 0;
    float totls = 0;
    int totlsI = 64;
    for (int i = 0; i < 64; i++)
    {
        float3 sp = normalize(attr.vec[i].xyz) * attr.vec[i].w;
        sp = mul(tbn, sp);
        if (dot(sp, normal.xyz) < 0)
            sp = -sp;
        
        float4 dpos = float4(pos, 1) + float4(sp, 0);
        float4 ndcr = mul(mvp, float4(dpos.xyz, 1.0));
        float3 ndc = ndcr.xyz / ndcr.w;

        float2 texcp = ndc.xy * 0.5 + 0.5;
        
        float4 refPosition = texPosition.Sample(sampPosition, texcp);
        if (refPosition.a <= 0.5)
        {
            totls += 1.0;
            continue;
        }
        float4 refDepthNdc = mul(mvp, float4(refPosition.xyz, 1.0));
        refDepthNdc.xyz /= refDepthNdc.w;
        
        float refw = refDepthNdc.w;
        
        if (ndcr.w > refw + 1e-2)
        {
            float rangeCheck = smoothstep(0.0, 1.0, 60.0 / abs(ndcr.z - refw));
            occls += 1.0 * rangeCheck;
        }
        totls += 1.0;
    }
    psOut.ao = float4(1.0 - occls / totls, 0, 0, 0);
    
    
    return psOut;
}