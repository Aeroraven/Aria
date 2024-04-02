struct VSInput
{
    [[vk::location(0)]] float4 position : POSITION0;
    [[vk::location(1)]] float4 normal : NORMAL0;
    [[vk::location(2)]] float4 texcoord : TEXCOORD0;
    [[vk::location(3)]] float4 tangent : TANGENT0;
    [[vk::location(4)]] float4 texIndices : TEXCOORD1;
};

struct VSOutput
{
    float4 position : SV_Position;
    [[vk::location(0)]] float4 texCoord : TEXCOORD0;
    [[vk::location(1)]] float4 normal : NORMAL0;
    [[vk::location(2)]] float4 rawPos : POSITION0;
    [[vk::location(3)]] float4 ndcPos : POSITION1;
    [[vk::location(4)]] float4 texIndices : TEXCOORD1;
    [[vk::location(5)]] float4 tangent : TANGENT0;
    [[vk::location(6)]] float4 bitangent : TANGENT1;
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
    float4 tangent = mul(cam.model, float4(vsIn.tangent.xyz, 0.0));
    float4 bitangent = float4(vsIn.tangent.w * cross(vsIn.normal.xyz, tangent.xyz), 0.0);
    tangent.w = vsIn.tangent.w;
    vsOut.texCoord = vsIn.texcoord;
    vsOut.normal = mul(cam.model, float4(vsIn.normal.xyz, 0));
    vsOut.rawPos = vsIn.position;
    vsOut.ndcPos = vsOut.position;
    vsOut.texIndices = vsIn.texIndices;
    vsOut.tangent = tangent;
    vsOut.bitangent = bitangent;
    return vsOut;
    
}