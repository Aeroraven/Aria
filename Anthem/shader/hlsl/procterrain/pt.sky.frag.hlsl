
struct VSOutput
{
    [[vk::location(0)]] float4 texCoord : TEXCOORD0;
    [[vk::location(1)]] float4 worldPos : POSITION0;
};

struct PSOutput
{
    float4 color : SV_Target0;
    float4 normal : SV_Target1;
    float4 position : SV_Target2;
    float4 tangent : SV_Target3;
};
TextureCube texSkybox : register(t0, space1);
SamplerState sampSkybox : register(s0, space1);

PSOutput main(VSOutput vsOut)
{
    PSOutput psOut;
    psOut.color = float4(texSkybox.Sample(sampSkybox, vsOut.texCoord.xyz).rgb, 2.0);
    psOut.normal = float4(0, 0, 0, 0);
    psOut.position = float4(vsOut.worldPos.xyz, 2);
    psOut.tangent = float4(0, 0, 0, 0);
    
    return psOut;
}