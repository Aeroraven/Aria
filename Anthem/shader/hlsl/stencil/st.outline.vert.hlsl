struct VSInput
{
    [[vk::location(0)]] float4 position : POSITION0;
    [[vk::location(1)]] float4 normal : NORMAL0;
    [[vk::location(2)]] float4 texcoord : TEXCOORD0;
};
struct VSOutput{
    float4 position : SV_Position;
    [[vk::location(0)]] float4 normal : POSITION0;
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
    float4 pos = mul(cam.proj, mul(cam.view, mul(cam.model, vsIn.position)));
    float4 norm = mul(cam.proj, mul(cam.view, mul(cam.model, vsIn.position+0.002*vsIn.normal)));
    vsOut.position = pos;
    vsOut.normal = norm;
    return vsOut;
}