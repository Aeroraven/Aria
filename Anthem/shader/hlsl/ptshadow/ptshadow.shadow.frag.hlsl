struct GSOutput
{
    [[vk::location(0)]] float4 orgPosition : POSITION0;
};

float main(GSOutput gsOut) : SV_Depth
{
    float3 lightPos = float3(70.0, 150.0, -100.0);
    float dist = distance(lightPos, gsOut.orgPosition.xyz)/1000.0;
    return dist;
}