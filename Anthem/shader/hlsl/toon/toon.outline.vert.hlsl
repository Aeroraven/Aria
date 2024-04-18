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
    [[vk::location(3)]] float4 color : COLOR0;
};

struct Camera
{
    float4x4 proj;
    float4x4 view;
    float4x4 model;
    float4 aspect;
};

ConstantBuffer<Camera> camera : register(b0, space0);
Texture2D texPbrBase[4] : register(t0, space1);
SamplerState sampPbrBase[4] : register(s0, space1);

float4 getBaseColor(uint baseId, float2 uv)
{
    return texPbrBase[baseId].SampleLevel(sampPbrBase[baseId], uv,0).rgba;
}


VSOutput main(VSInput input)
{
    VSOutput output;
    output.position = mul(camera.proj, mul(camera.view, mul(camera.model, input.position )));
    float4 vNorm = mul(camera.model, float4(input.normal.xyz, 0.0));
    float4 extDir = mul(camera.proj, mul(camera.view, vNorm));
    float3 extDir3 = normalize(extDir.xyz);
    output.position.xy += 0.0020 * extDir3.xy * output.position.w * float2(1.0, camera.aspect.r);
    
    output.texUv = input.uv;
    output.normal = mul(camera.model, input.normal);
    output.texIndices = input.texIndices;
    output.color = getBaseColor(uint(input.texIndices.r), input.uv.xy);
    return output;
}
