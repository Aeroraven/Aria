struct VSInput
{
    [[vk::location(0)]] float4 position : POSITION0;
    [[vk::location(1)]] float4 normal : NORMAL0;
    [[vk::location(2)]] float4 texcoord : TEXCOORD0;
};

struct VSOutput
{
    float4 position : SV_Position;
};
VSOutput main(VSInput vsIn)
{
    VSOutput vsOut;
    vsOut.position = float4(vsIn.position.xyz, 1.0);
    return vsOut;
}