#include "pt.common.hlsl"

struct VSInput
{
    [[vk::location(0)]] float4 position : POSITION0;
};

struct VSOutput
{
    float4 position : SV_Position;
    [[vk::location(0)]] float4 texCoord : TEXCOORD0;
};


ConstantBuffer<Camera> cam : register(b0, space0);


VSOutput main(VSInput vsIn)
{
    VSOutput vsOut;
    float3 pos = vsIn.position.xyz;
    vsOut.position = mul(cam.proj, mul(cam.view, mul(cam.model, float4(pos, 1.0f) + float4(0, 50, 0,0))));
    vsOut.texCoord = float4(vsIn.position.xyz, 1);
    return vsOut;
    
}