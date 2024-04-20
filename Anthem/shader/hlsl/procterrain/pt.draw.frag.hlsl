
struct VSOutput
{
    [[vk::location(0)]] float4 normal : NORMAL0;
};

float4 main(VSOutput vsOut) : SV_Target0
{
    return float4(vsOut.normal.xyz, 1.0f)*0.8;
}