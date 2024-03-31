struct VSOutput
{
    [[vk::location(0)]] float2 texCoord : TEXCOORD0;
};

Texture2D texIn : register(t0, space0);
SamplerState sampIn : register(s0, space0);

float4 main(VSOutput vsOut) : SV_Target0
{
    return texIn.Sample(sampIn, vsOut.texCoord);
}