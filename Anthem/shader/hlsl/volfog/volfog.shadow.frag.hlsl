struct VSOutput
{
    [[vk::location(0)]] float4 texCoord : TEXCOORD0;
    [[vk::location(1)]] float4 normal : NORMAL0;
};

struct PSOutput
{
    float4 color : SV_Target0;
};

Texture2D texPbrBase[25] : register(t0, space1);
SamplerState sampPbrBase[25] : register(s0, space1);

PSOutput main(VSOutput vsOut)
{
    PSOutput psOut;
    int texId = int(vsOut.texCoord.a);
    psOut.color = texPbrBase[texId].Sample(sampPbrBase[texId], vsOut.texCoord.xy);
    return psOut;
}