struct VSInput
{
    [[vk::location(0)]] float4 pos : POSITION0;
    [[vk::location(1)]] float4 norm : NORMAL0;
    [[vk::location(2)]] float4 tex : TEXCOORD0;
};

struct VSOutput
{
    float4 ndcPos : SV_Position;
    [[vk::location(0)]] float4 worldPos : POSITION0;
    [[vk::location(1)]] float3 normal : NORMAL0;
    [[vk::location(2)]] float inst : POSITION1;
};

VSOutput main(VSInput vsIn)
{
    VSOutput vsOut;
    return vsOut;
}