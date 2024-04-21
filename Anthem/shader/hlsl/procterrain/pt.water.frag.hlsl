struct PSOutput
{
    float4 color : SV_Target0;
    float4 normal : SV_Target1;
    float4 position : SV_Target2;
    float4 tangent : SV_Target3;
};


struct VSOutput
{
    [[vk::location(0)]] float4 normal : NORMAL0;
    [[vk::location(1)]] float4 position : POSITION0;
    [[vk::location(2)]] float4 tangent : TANGENT0;
};


PSOutput main(VSOutput vsOut)
{
    PSOutput psOut;
    psOut.color = float4(0.2, 0.2, 1.0, 1.0);
    psOut.normal = vsOut.normal;
    psOut.position = vsOut.position;
    psOut.tangent = vsOut.tangent;
    return psOut;
}