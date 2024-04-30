struct VSOutput
{
    [[vk::location(0)]] float4 TexCoord : TEXCOORD0;
};

struct PSOutput{
    float4 color : SV_TARGET0;
};

Texture2D texDye: register(t0,space0);
SamplerState samLinear: register(s0,space0);


float4 gammaCorrect(float4 color)
{
    return pow(color, 1.0/2.2);
}
PSOutput main(VSOutput input)
{
    PSOutput output;
    float texH, texW;
    texH = 1.0/1024.0;
    texW = 1.0/1024.0;
    float2 delta = float2(texW, texH);
    output.color = texDye.Sample(samLinear, input.TexCoord.xy);
    output.color = gammaCorrect(output.color);
    output.color.w = max(output.color.r, max(output.color.g, output.color.b))-0.6;
    return output;
}