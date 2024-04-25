#include "pt.common.hlsl"
struct VSInput
{
    [[vk::location(0)]] float4 position : POSITION0;
    [[vk::location(1)]] float4 normal : NORMAL0;
    [[vk::location(2)]] float4 texcoord : TEXCOORD0;
    [[vk::location(3)]] float4 tangent : TANGENT0;
    [[vk::location(4)]] float4 texIndices : TEXCOORD1;
    [[vk::location(5)]] float4 instancePos : POSITION1;
};

struct VSOutput
{
    float4 position : SV_Position;
    [[vk::location(0)]] float4 normal : NORMAL0;
    [[vk::location(1)]] float4 rawPosition : POSITION0;
    [[vk::location(2)]] float4 tangent : TANGENT0;
    [[vk::location(3)]] float4 instancePos : POSITION1;
    [[vk::location(4)]] float4 texIndices : POSITION2;
    [[vk::location(5)]] float4 uv : TEXCOORD0;
};
struct LocalMove
{
    float4x4 local;
};

[[vk::push_constant]] LocalMove localMove;
ConstantBuffer<Camera> cam : register(b0, space0);

VSOutput main(VSInput vsIn)
{
    VSOutput vsOut;
    float4 localpos = mul(localMove.local, float4(vsIn.position.xyz, 1));
    
    vsOut.position = mul(cam.proj, mul(cam.view, mul(cam.model, float4((localpos + vsIn.instancePos).xyz, 1))));
    vsOut.normal = mul(cam.model, float4(vsIn.normal.xyz, 0.0));
    vsOut.rawPosition = float4((localpos + vsIn.instancePos).xyz, 1);
    vsOut.tangent = mul(cam.model, float4(vsIn.tangent.xyz, 0.0));
    vsOut.instancePos = vsIn.instancePos;
    vsOut.texIndices = vsIn.texIndices;
    vsOut.uv = vsIn.texcoord;
    return vsOut;
}

