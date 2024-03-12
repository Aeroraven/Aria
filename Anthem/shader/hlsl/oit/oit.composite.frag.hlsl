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
    float4 solid = texSolid.Sample(sampSolid, vsOut.texCoord);
    float4 accum = texAccum.Sample(sampAccum, vsOut.texCoord);
    float4 reveal = texReveal.Sample(sampReveal, vsOut.texCoord);
    
    float4 trans = float4(accum.rgb / max(accum.a, 0.05), reveal.r);
    float4 color = (1.0 - reveal.r) * trans + reveal.r * solid;
    psOut.color = color;
    return psOut;
}