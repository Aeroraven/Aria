struct VSOutput
{
    [[vk::location(0)]] float2 texCoord : TEXCOORD0;
};

Texture2D texIn : register(t0, space0);
SamplerState sampIn : register(s0, space0);

float4 simpleBlur(float2 uv)
{
    float texH, texW, texLod;
    texIn.GetDimensions(0, texW, texH, texLod);
    float2 texelSize = 1.0 / float2(texW, texH);
    
    float4 color = float4(0, 0, 0, 0);
    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            color += texIn.Sample(sampIn, uv + float2(x, y) * texelSize);
        }
    }
    return color / 9.0;
}

float4 main(VSOutput vsOut) : SV_Target0
{
    return simpleBlur(vsOut.texCoord.xy);
}
