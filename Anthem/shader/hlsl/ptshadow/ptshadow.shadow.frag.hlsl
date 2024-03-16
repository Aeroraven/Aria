struct GSOutput
{
    [[vk::location(0)]] float4 orgPosition : POSITION0;
};

float4 main(GSOutput gsOut) : SV_TARGET
{
    float3 lightPos = float3(80.0, 150.0, -80.0);
    float dist = distance(lightPos, gsOut.orgPosition.xyz);
    return float4(0.0, 0.0, 0.0, 1.0);
}