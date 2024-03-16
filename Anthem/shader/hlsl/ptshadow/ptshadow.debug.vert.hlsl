struct VSInput
{
    [[vk::location(0)]] float4 position : POSITION0;
    [[vk::location(1)]] float4 normal : NORMAL0;
    [[vk::location(2)]] float4 texcoord : TEXCOORD0;
};

struct VSOutput
{
    float4 position : SV_Position;
    [[vk::location(0)]] float4 texCoord : TEXCOORD0;
};

struct Camera
{
    float4x4 proj;
    float4x4 view;
    float4x4 model;
};

cbuffer UniformBuffer : register(b0, space0)
{
    Camera cam;
}
ConstantBuffer<Camera> lcm[6] : register(b0, space2);

VSOutput main(VSInput vsIn)
{
    VSOutput vsOut;
    uint p = 4;
    vsOut.position = mul(cam.proj, mul(cam.view, mul(cam.model, vsIn.position)));
    vsOut.texCoord = vsIn.position;
    return vsOut;
    
}