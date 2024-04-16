struct VSOutput
{
    [[vk::location(0)]] float4 texUv: TEXCOORD0;
    [[vk::location(1)]] float4 normal: NORMAL0;
    [[vk::location(2)]] float4 texIndices : TEXCOORD1;
};
Texture2D texPbrBase[4] : register(t0, space1);
SamplerState sampPbrBase[4] : register(s0, space1);

float4 getBaseColor(uint baseId, float2 uv)
{
    return texPbrBase[baseId].Sample(sampPbrBase[baseId], uv).rgba;
}

float4 main(VSOutput vsOut) : SV_Target
{
    //return float4(0.8, 0.8, 0.8, 1);
    return getBaseColor(uint(vsOut.texIndices.r), vsOut.texUv.xy);
}
