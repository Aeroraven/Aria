struct VSOutput
{
    [[vk::location(0)]] float4 texUv: TEXCOORD0;
    [[vk::location(1)]] float4 normal: NORMAL0;
    [[vk::location(2)]] float4 texIndices : TEXCOORD1;
    [[vk::location(3)]] float4 color : COLOR0;
};
Texture2D texPbrBase[4] : register(t0, space1);
SamplerState sampPbrBase[4] : register(s0, space1);

float4 getBaseColor(uint baseId, float2 uv)
{
    return texPbrBase[baseId].Sample(sampPbrBase[baseId], uv).rgba;
}

float4 gamma(float4 color)
{
    return pow(color, 1.0 / 2.2);
}
float4 main(VSOutput vsOut) : SV_Target
{
    return gamma(lerp(float4(0, 0, 0, 1), vsOut.color, 0.0));

}
