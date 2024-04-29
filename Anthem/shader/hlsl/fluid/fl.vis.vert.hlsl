#include "fl.common.hlsl"

struct VSInput
{
    [[vk::location(0)]] float4 Position : POSITION;
    [[vk::location(1)]] float4 TexCoord : TEXCOORD0;
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    [[vk::location(0)]] float4 TexCoord : TEXCOORD0;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    output.Position = input.Position;
    output.TexCoord = input.TexCoord;
    return output;
}