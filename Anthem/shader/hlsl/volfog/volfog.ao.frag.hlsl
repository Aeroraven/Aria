struct VSOutput
{
    [[vk::location(0)]] float4 texCoord : TEXCOORD0;
};
struct AOParams
{
    float4 vec[64];
};
struct Camera
{
    float4x4 proj;
    float4x4 view;
    float4x4 model;
};
ConstantBuffer<AOParams> attr : register(b0, space0);
Texture2D texBaseColor : register(t0, space1);
SamplerState sampBaseColor : register(s0, space1);
Texture2D texNorm : register(t0, space2);
SamplerState sampNorm : register(s0, space2);
Texture2D texPos : register(t0, space3);
SamplerState sampPos : register(s0, space3);
Texture2D texNdc : register(t0, space4);
SamplerState sampNdc : register(s0, space4);
Texture2D texTangent : register(t0, space5);
SamplerState sampTangent : register(s0, space5);

ConstantBuffer<Camera> cam : register(b0, space6);

struct PSOutput
{
    float4 ao : SV_Target0;
};

PSOutput main(VSOutput vsOut)
{
    PSOutput psOut;
    float4 pos = texPos.Sample(sampPos, vsOut.texCoord.xy);
    float4 adjNormal = texNorm.Sample(sampNorm, vsOut.texCoord.xy);
    float4 tangent = texTangent.Sample(sampTangent, vsOut.texCoord.xy);
    float3 bitangent = cross(normalize(tangent.xyz), normalize(adjNormal.xyz));
    float3x3 tbn = transpose(float3x3(normalize(tangent.xyz), bitangent, normalize(adjNormal.xyz)));
    
    float occls = 0;
    float totls = 0;
    for (int i = 0; i < 64; i++)
    {
        float3 sp = normalize(attr.vec[i].xyz)*attr.vec[i].w;
        sp = mul(tbn, sp);
        if (dot(sp, adjNormal.xyz) < 0)
            sp = -sp;
        
        float4 dpos = pos + float4(sp, 0);
        float4 ndcr = mul(cam.proj, mul(cam.view, mul(cam.model, float4(dpos.xyz, 1.0))));
        float3 ndc = ndcr.xyz / ndcr.w;

        float2 texcp = ndc.xy * 0.5 + 0.5;
        
        float4 refDepthNdc = texNdc.Sample(sampNdc, texcp);
        float refw =  refDepthNdc.w;
        
        if (ndcr.w > refw  + 1e-2)
        {
            occls += 1.0;
        }
        totls += 1.0;
    }
    psOut.ao = float4(occls / totls, occls / totls, occls / totls, 0);
    return psOut;
}