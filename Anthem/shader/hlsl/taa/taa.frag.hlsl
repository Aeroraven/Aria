struct VSOutput
{
    [[vk::location(0)]] float2 texCoord : TEXCOORD0;
};

Texture2D texTarget : register(t0, space0);
SamplerState sampTarget : register(s0, space0);
Texture2D texHistory : register(t0, space1);
SamplerState sampHistory : register(s0, space1);
Texture2D texMotion : register(t0, space2);
SamplerState sampMotion : register(s0, space2);

float4 main(VSOutput vsOut) : SV_Target0
{
    return texMotion.Sample(sampMotion, vsOut.texCoord);
    //return lerp(texHistory.Sample(sampHistory, vsOut.texCoord), texTarget.Sample(sampTarget, vsOut.texCoord), 0.05);
}