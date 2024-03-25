struct VSOutput
{
    [[vk::location(0)]] float4 orgPosition : POSITION0;
    [[vk::location(1)]] float4 texCoord : TEXCOORD0;
    [[vk::location(2)]] float4 normal : NORMAL0;
    [[vk::location(3)]] float4 legacyPositionRaw : POSITION1;
    [[vk::location(4)]] float4 curPositionRaw : POSITION2;
};

struct Camera
{
    float4x4 proj;
    float4x4 view;
    float4x4 model;
};

Texture2D texImage[30] : register(t0, space1);
SamplerState sampImage[30] : register(s0, space1);

struct PSOutput
{
    float4 color : SV_Target0;
    float4 motionVec : SV_Target1;
};

PSOutput main(VSOutput vsOut)
{
    PSOutput psOut;
    float4 baseColor = float4(1.0, 1.0, 1.0, 1.0);
    int index = int(vsOut.texCoord.z);
    baseColor = texImage[index].Sample(sampImage[index], vsOut.texCoord.xy);
    psOut.color = baseColor;
    
    float4 lastPos = vsOut.legacyPositionRaw / vsOut.legacyPositionRaw.w;
    float4 curPos = vsOut.curPositionRaw / vsOut.curPositionRaw.w;
    float2 offset = (curPos - lastPos).xy;
    psOut.motionVec = float4((abs(offset) * 0.5 + 0.5)*100, 0, 0);
    
    return psOut;
}