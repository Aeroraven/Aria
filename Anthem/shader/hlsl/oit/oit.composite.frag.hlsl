struct VSOutput
{
    [[vk::location(0)]] float2 texCoord : TEXCOORD0;
};

struct PSOutput
{
    float4 color : SV_Target0;
};

Texture2D texAccum : register(t0, space0);
SamplerState sampAccum : register(s0, space0);
Texture2D texReveal : register(t0, space1);
SamplerState sampReveal : register(s0, space1);
Texture2D texSolid : register(t0, space2);
SamplerState sampSolid : register(s0, space2);

PSOutput main(VSOutput vsOut)
{
    PSOutput psOut;
    psOut.color = float4(1.0, 0.0, 0.0, 1.0);
    return psOut;
}