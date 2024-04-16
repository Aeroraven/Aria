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
    output.position = mul(camera.proj, mul(camera.view, mul(camera.model, input.position )));
    float4 vNorm = mul(camera.model, float4(input.normal.xyz, 0.0));
    float4 extDir = mul(camera.proj, vNorm);
    float3 extDir3 = normalize(extDir.xyz);
    extDir3.xy = normalize(extDir3.xy);
    output.position += 0.0025 * camera.aspect.r * float4(extDir3, 0);
    
    output.texUv = input.uv;
    output.normal = mul(camera.model, input.normal);
    output.texIndices = input.texIndices;
    return output;
}
