struct VSInput
{
    [[vk::location(0)]] float4 position : POSITION0;
    [[vk::location(1)]] float4 normal : NORMAL0;
    [[vk::location(2)]] float4 uv : TEXCOORD0;
    [[vk::location(3)]] float4 tangent : TANGENT0;
    [[vk::location(4)]] float4 texIndices : TEXCOORD1;
};

struct VSOutput
{
    float4 position : SV_Position;
    [[vk::location(0)]] float4 texUv : TEXCOORD0;
    [[vk::location(1)]] float4 normal : NORMAL0;
    [[vk::location(2)]] float4 texIndices : TEXCOORD1;
};

struct Camera
{
    float4x4 proj;
    float4x4 view;
    float4x4 model;
    float4 aspect;
};

ConstantBuffer<Camera> camera : register(b0, space0);

VSOutput main(VSInput input)
{
    VSOutput output;
    output.position = mul(camera.proj, mul(camera.view, mul(camera.model, input.position)));
    output.texUv = input.uv;
    output.normal = mul(camera.model, input.normal);
    output.texIndices = input.texIndices;
    
    return output;
}
