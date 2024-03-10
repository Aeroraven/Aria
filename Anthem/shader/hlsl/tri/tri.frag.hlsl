struct FSInput
{
    [[vk::location(0)]] float4 Color : COLOR0;
};

float4 main(FSInput sIn) : SV_Target
{
    return float4(sIn.Color.xyz, 1.0);
}