#include "./sine.general.hlsl"

struct VSInput
{
    [[vk::location(0)]] float4 position : POSITION0;
};

struct VSOutput
{
    float4 position : SV_Position;
    [[vk::location(0)]] float4 texCoord : TEXCOORD0;
};

cbuffer UniformBuffer : register(b0, space0)
{
    Constants cam;
}

VSOutput main(VSInput vsIn)
{
    VSOutput vsOut;
    float3 pos = vsIn.position.xyz + cam.camPos.xyz;
    vsOut.position = mul(cam.proj, mul(cam.view, mul(cam.model, float4(pos, 1.0f))));
    vsOut.texCoord = vsIn.position;
    return vsOut;
    
}