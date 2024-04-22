struct PSOutput
{
    float4 mask : SV_Target0;
    float4 texUv : SV_Target1;
};

struct VSOutput
{
    [[vk::location(0)]] float4 normal : NORMAL0;
    [[vk::location(1)]] float4 position : POSITION0;
    [[vk::location(2)]] float4 tangent : TANGENT0;
    [[vk::location(3)]] float4 uv : TEXCOORD0;
};


PSOutput main(VSOutput vsOut)
{
    PSOutput psOut;
    psOut.mask = float4(vsOut.position.xyz, 1.0);
    psOut.texUv = float4(vsOut.uv.xy, 0, 0);
    return psOut;
}