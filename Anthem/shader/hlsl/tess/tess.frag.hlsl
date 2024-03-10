struct DSOutput
{
    [[vk::location(0)]] float4 color : COLOR0;
};

float4 main(DSOutput dsOut) : SV_Target
{
    return float4(1.0, 0.0, 0.0, 1.0);
}