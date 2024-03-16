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

struct Camera
{
    float4x4 proj;
    float4x4 view;
    float4x4 model;
};

cbuffer Uniform : register(b0, space0)
{
    Camera cam;
}

VSOutput main(VSInput vsIn)
{
    VSOutput vsOut;
    vsOut.position = mul(cam.proj, mul(cam.view, mul(cam.model, vsIn.position)));
    return vsOut;
}