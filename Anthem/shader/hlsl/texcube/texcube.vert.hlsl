struct VSInput
{
    [[vk::location(0)]] float4 position : POSITION0;
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

VSOutput main(VSInput vsIn)
{
    VSOutput vsOut;
    vsOut.position = mul(cam.proj, mul(cam.view, mul(cam.model, vsIn.position)));
    vsOut.texCoord = vsIn.position;
    return vsOut;
    
}