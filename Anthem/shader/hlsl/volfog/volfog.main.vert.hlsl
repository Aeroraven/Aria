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
    [[vk::location(1)]] float4 normal : NORMAL0;
    [[vk::location(2)]] float4 rawPos : POSITION0;
};

struct Camera
{
    float4x4 proj;
    float4x4 view;
    float4x4 model;
};

ConstantBuffer<Camera> cam : register(b0, space0);

VSOutput main(VSInput vsIn)
{
    VSOutput vsOut;
    vsOut.position = mul(cam.proj, mul(cam.view, mul(cam.model, float4(vsIn.position.xyz, 1.0))));
    vsOut.texCoord = vsIn.texcoord;
    vsOut.normal = vsIn.normal;
    vsOut.rawPos = vsIn.position;
    return vsOut;
    
}