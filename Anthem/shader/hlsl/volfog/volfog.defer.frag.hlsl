struct VSOutput
{
    [[vk::location(0)]] float4 texCoord : TEXCOORD0;
    [[vk::location(1)]] float4 normal : NORMAL0;
    [[vk::location(2)]] float4 rawPos : POSITION0;
    [[vk::location(3)]] float4 ndcPos : POSITION1;
    [[vk::location(4)]] float4 texIndices : TEXCOORD1;
    [[vk::location(5)]] float4 tangent : TANGENT0;
    [[vk::location(6)]] float4 bitangent : TANGENT1;
};

struct PSOutput
{
    float4 color : SV_Target0;
    float4 norm : SV_Target1;
    float4 pos : SV_Target2;
    float4 ndc : SV_Target3;
    float4 tangent : SV_Target4;
};


Texture2D texPbrBase[69] : register(t0, space1);
SamplerState sampPbrBase[69] : register(s0, space1);


float3 getNormal(uint normalMapId, float2 uv, float4 t, float4 n,out float4 oTangent,out float4 oBitangent)
{
    float3 normalCx = texPbrBase[normalMapId].Sample(sampPbrBase[normalMapId], uv).rgb;
    normalCx = normalize(normalCx * 2.0 - 1.0);
    float3 N = normalize(n.xyz);
    float3 Tw = normalize(t.xyz);
    float3 T = normalize(Tw);
    oTangent = float4(T, 1.0);
    float3 B = cross(N, T) * t.w;
    oBitangent = float4(B, 1.0);
    float3x3 tbn = float3x3(T, B, N);
    return normalize(mul(normalCx, tbn));
}

float4 getBaseColor(uint baseId, float2 uv)
{
    return texPbrBase[baseId].Sample(sampPbrBase[baseId], uv).rgba;
}
PSOutput main(VSOutput vsOut)
{
    PSOutput psOut;
    float4 tangent = vsOut.tangent;
    float4 normal = vsOut.normal;
    float4 oTangent;
    float4 oBitangent;
    float4 adjNormal = float4(getNormal(uint(vsOut.texIndices.g), vsOut.texCoord.xy, tangent, normal, oTangent, oBitangent), 1.0);
    
    psOut.color = getBaseColor(uint(vsOut.texIndices.r), vsOut.texCoord.xy);
    psOut.norm = adjNormal; 
    psOut.pos = vsOut.rawPos;
    psOut.ndc = vsOut.ndcPos;
    psOut.tangent = vsOut.tangent;
    return psOut;
}