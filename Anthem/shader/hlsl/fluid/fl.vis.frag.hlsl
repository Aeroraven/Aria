struct VSOutput
{
    [[vk::location(0)]] float4 TexCoord : TEXCOORD0;
};

struct PSOutput{
    float4 color : SV_TARGET0;
};

Texture2D texDye: register(t0,space0);
SamplerState samLinear: register(s0,space0);

PSOutput main(VSOutput input)
{
    PSOutput output;
    output.color = texDye.Sample(samLinear, input.TexCoord.xy);
    return output;
}