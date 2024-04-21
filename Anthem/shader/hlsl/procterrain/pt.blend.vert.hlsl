struct VSInput
{
    [[vk::location(0)]] float4 position : POSITION0;
    [[vk::location(1)]] float4 texCoord : TEXCOORD0;
};

struct VSOutput
{
    float4 position : SV_Position;
    [[vk::location(0)]] float4 texCoord : TEXCOORD0;
};


VSOutput main(VSInput vsIn)
{
    VSOutput vsOut;
    vsOut.position = vsIn.position;
    vsOut.texCoord = vsIn.texCoord;
    return vsOut;
    
}