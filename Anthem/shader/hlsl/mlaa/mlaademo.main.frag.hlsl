struct VSOutput
{
    [[vk::location(0)]] float4 orgPosition : POSITION0;
    [[vk::location(1)]] float4 texCoord : TEXCOORD0;
    [[vk::location(2)]] float4 normal : NORMAL0;
};

struct Camera
{
    float4x4 proj;
    float4x4 view;
    float4x4 model;
};

struct Light
{
    float4 pos;
    float4 ambient;
};


Texture2D texImage[5] : register(t0, space1);
SamplerState sampImage[5] : register(s0, space1);


float4 main(VSOutput vsOut) : SV_Target
{
    
    float4 baseColor = float4(1.0, 1.0, 1.0, 1.0);
    int index = int(vsOut.texCoord.w);
    baseColor = texImage[index].Sample(sampImage[index], vsOut.texCoord.xy);
    return baseColor;
}