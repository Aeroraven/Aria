struct PSOutput
{
    float4 color : SV_Target0;
    float4 normal : SV_Target1;
    float4 position : SV_Target2;
    float4 tangent : SV_Target3;
};

struct VSOutput
{
    float4 position : SV_Position;
    [[vk::location(0)]] float4 normal : NORMAL0;
    [[vk::location(1)]] float4 rawPosition : POSITION0;
    [[vk::location(2)]] float4 tangent : TANGENT0;
    [[vk::location(3)]] float4 instancePos : POSITION1;
    [[vk::location(4)]] float4 texIndices : POSITION2;
};

Texture2D tex[5] : register(t0, space1);
SamplerState samp[5] : register(s0, space1);

PSOutput main(VSOutput vsOut)
{
    PSOutput psOut;
    int baseColorIndex = int(vsOut.texIndices.r);
    psOut.color = tex[baseColorIndex].Sample(samp[baseColorIndex], vsOut.texIndices.xy);
    psOut.normal = float4(0, 1, 0, 0);
    psOut.position = vsOut.rawPosition;
    psOut.tangent = vsOut.tangent;   
    return psOut;
}
