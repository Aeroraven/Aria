struct VSInput
{
    [[vk::location(0)]] float4 position : POSITION0;
    [[vk::location(1)]] float4 normal : NORMAL0;
    [[vk::location(2)]] float4 texcoord : TEXCOORD0;
};

struct VSOutput
{
    float4 position : SV_Position;
    [[vk::location(0)]] float4 orgPosition : POSITION0;
    [[vk::location(1)]] float4 texCoord : TEXCOORD0;
    [[vk::location(2)]] float4 normal : NORMAL0;
    [[vk::location(3)]] float4 legacyPositionRaw : POSITION1;
    [[vk::location(4)]] float4 curPositionRaw : POSITION2;
};

struct Camera
{
    float4x4 projOld;
    float4x4 viewOld;
    float4x4 modelOld;
    float4x4 proj;
    float4x4 view;
    float4x4 model;
    float4x4 projRaw;
};

ConstantBuffer<Camera> cam : register(b0, space0);


VSOutput main(VSInput vsIn)
{
    VSOutput vsOut;
    vsOut.position = mul(cam.proj, mul(cam.view, mul(cam.model, vsIn.position)));
    vsOut.normal = mul(cam.model, float4(vsIn.normal.xyz, 0.0));
    vsOut.texCoord = vsIn.texcoord;
    vsOut.orgPosition = mul(cam.model, vsIn.position);
    
    vsOut.legacyPositionRaw = mul(cam.projRaw, mul(cam.viewOld, mul(cam.modelOld, vsIn.position)));
    vsOut.curPositionRaw = mul(cam.projRaw, mul(cam.view, mul(cam.model, vsIn.position)));
    
    return vsOut;
}