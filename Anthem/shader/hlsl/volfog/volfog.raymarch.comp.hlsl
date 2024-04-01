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
static const float SLICE_TRANS_FACTOR = 0.1f;

ConstantBuffer<LightAttrs> attr : register(b0, space0);
RWTexture3D<float4> fogData : register(u0, space1);
RWTexture3D<float4> scatData : register(u0, space2);

float sliceZLength(uint2 invXY, uint invZ)
{
    uint3 tInvIdN = uint3(invXY, invZ);
    uint3 tInvIdF = uint3(invXY, invZ + 1);
    float3 wPN = invId2WorldPos(tInvIdN, attr.volSize.xyz, attr.inverseVp, attr.farNear.x, attr.farNear.y).xyz;
    float3 wPF = invId2WorldPos(tInvIdF, attr.volSize.xyz, attr.inverseVp, attr.farNear.x, attr.farNear.y).xyz;
    return abs(wPF.z - wPN.z);
}
void rayStep(uint2 invXY,uint invZ ,inout float accumTrans, inout float3 accumColor, float scatterDensity, float3 scatterColor)
{
    float thickness = (attr.farNear.x - attr.farNear.y) / float(attr.volSize.z); //sliceZLength(invXY, invZ);
    float sliceTrans = exp(-thickness * scatterDensity * SLICE_TRANS_FACTOR);
    float3 sliceColor = (1 - sliceTrans) * scatterColor;
    float3 contribColor = sliceColor * accumTrans;
    accumColor += contribColor;
    accumTrans *= sliceTrans;
}

[numthreads(8, 8, 1)]
void main(uint3 invId : SV_DispatchThreadID)
{
    float3 accumColor = float3(0, 0, 0);
    float accumTrans = 1;
    for (int i = 0; i < attr.volSize.z; i++)
    {
        uint3 tInvId = uint3(invId.xy, uint(i));
        float3 worldPos = invId2WorldPos(tInvId, attr.volSize.xyz, attr.inverseVp, attr.farNear.x, attr.farNear.y).xyz;
        float4 scatVoxel = scatData[tInvId];
        float scatDensity = scatVoxel.a;
        float3 scatColor = scatVoxel.rgb;
        rayStep(invId.xy, uint(i), accumTrans, accumColor, scatDensity, scatColor);
        
        fogData[tInvId] = float4(accumColor, accumTrans);
    }
}