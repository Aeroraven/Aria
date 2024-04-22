#include "pt.common.hlsl"

struct VSInput
{
    [[vk::location(0)]] float4 position : POSITION0;
    [[vk::location(1)]] float4 normal : NORMAL0;
    [[vk::location(2)]] float4 tangent : TANGENT0;
    [[vk::location(3)]] float4 uv : TEXCOORD0;
};

struct VSOutput
{
    float4 position : SV_Position;
    [[vk::location(0)]] float4 normal : NORMAL0;
    [[vk::location(1)]] float4 rawPosition : POSITION0;
    [[vk::location(2)]] float4 tangent : TANGENT0;
    [[vk::location(3)]] float4 uv : TEXCOORD0;
};


ConstantBuffer<Camera> cam : register(b0, space0);

VSOutput main(VSInput vsIn)
{
    VSOutput vsOut;
    vsOut.position = mul(cam.proj, mul(cam.view, mul(cam.model, float4(vsIn.position.xyz, 1))));
    vsOut.normal = mul(cam.model, float4(vsIn.normal.xyz, 0.0));
    vsOut.rawPosition = vsIn.position;
    vsOut.tangent = mul(cam.model, float4(vsIn.tangent.xyz, 0.0));
    vsOut.uv = vsIn.uv;
    return vsOut;
}