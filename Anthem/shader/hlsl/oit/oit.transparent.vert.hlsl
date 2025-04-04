struct VSInput
{
    [[vk::location(0)]] float4 position : POSITION0;
    [[vk::location(1)]] float4 color : COLOR0;
};


struct Camera
{
    float4x4 proj;
    float4x4 view;
    float4x4 model;
};
[[vk::push_constant]] Camera cam;

struct VSOutput
{
    float4 position : SV_Position;
    [[vk::location(0)]] float4 color : COLOR0;
    [[vk::location(1)]] float posz : POSITION0;
};

VSOutput main(VSInput vsIn)
{
    VSOutput vsOut;
    vsOut.position = mul(cam.proj, mul(cam.view, mul(cam.model, float4(vsIn.position.xyz, 1.0))));
    vsOut.color = vsIn.color;
    vsOut.posz = vsOut.position.z / vsOut.position.w;
    return vsOut;
}