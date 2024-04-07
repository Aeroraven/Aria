struct VSInput
{
    [[vk::location(0)]] float4 pos : POSITION0;
    [[vk::location(1)]] float4 norm : NORMAL0;
    [[vk::location(2)]] float4 tex : TEXCOORD0;
};

struct VSOutput
{
    float4 ndcPos : SV_Position;
    [[vk::location(0)]] float4 worldPos : POSITION0;
    [[vk::location(1)]] float3 normal : NORMAL0;
    [[vk::location(2)]] float inst : POSITION1;
    [[vk::location(3)]] float2 tex : TEXCOORD0;
};

struct Camera
{
    float4x4 proj;
    float4x4 view;
    float4x4 model;
    float4 camPos;
};

ConstantBuffer<Camera> cam : register(b0, space0);

VSOutput main(VSInput vsIn)
{
    VSOutput vsOut;
    vsOut.ndcPos = mul(cam.proj, mul(cam.view, mul(cam.model, vsIn.pos)));
    vsOut.worldPos = mul(cam.model, vsIn.pos);
    vsOut.normal = vsIn.norm.xyz;
    vsOut.inst = vsIn.tex.z;
    vsOut.tex = vsIn.tex.xy;
    return vsOut;
}