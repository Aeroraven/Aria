struct DSOutput
{
    [[vk::location(0)]] float4 color : COLOR0;
};

float4 main(DSOutput dsOut) : SV_Target
{
    return dsOut.color;
}