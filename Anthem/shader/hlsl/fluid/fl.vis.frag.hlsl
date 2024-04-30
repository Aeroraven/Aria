struct VSOutput
{
    [[vk::location(0)]] float4 TexCoord : TEXCOORD0;
};

struct PSOutput{
    float4 color : SV_TARGET0;
};

Texture2D texDye: register(t0,space0);
SamplerState samLinear: register(s0,space0);

float3 hsv2rgb(float3 c)
{
    // Reference: http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
    float4 K = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    float3 p = abs(frac(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * lerp(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

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
    return output;
}