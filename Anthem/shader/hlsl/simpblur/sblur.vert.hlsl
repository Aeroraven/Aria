
struct VSInput
{
    [[vk::location(0)]] float4 position : POSITION0;
};

struct VSOutput
{
    float4 position : SV_Position;
    [[vk::location(0)]] float2 texCoord : TEXCOORD0;
};

VSOutput main(VSInput vsIn)
{
    VSOutput vsOut;
    vsOut.position = vsIn.position;
    vsOut.texCoord = vsIn.position.xy * 0.5 + 0.5;
    return vsOut;
}