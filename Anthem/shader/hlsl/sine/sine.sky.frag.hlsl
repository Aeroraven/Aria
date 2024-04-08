#include "./sine.general.hlsl"

struct VSOutput
{
    [[vk::location(0)]] float4 texCoord : TEXCOORD0;
};

struct PSOutput
{
    float4 color : SV_Target0;
};

TextureCube texSkybox : register(t0, space1);
SamplerState sampSkybox : register(s0, space1);

PSOutput main(VSOutput vsOut)
{
    PSOutput psOut;
    psOut.color = float4(texSkybox.Sample(sampSkybox, vsOut.texCoord.xyz).rgb, 1.0);
    return psOut;
}