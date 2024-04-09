#include "./sine.general.hlsl"

struct VSInput
{
    [[vk::location(0)]] float4 pos : POSITION0;
};

struct VSOutput
{
    float4 ndcPos : SV_Position;
    [[vk::location(0)]] float4 pos : POSITION0;
};

ConstantBuffer<Constants> data : register(b0, space0);

VSOutput main(VSInput vsIn)
{
    VSOutput vsOut;
    float2 p = vsIn.pos.xz;
    float3 fv = calcWave(p, data.timing.x, data.wave, data.freqAmpl.y, data.freqAmpl.x, data.freqAmpl.w, data.freqAmpl.z, 
        data.warpWaves.x, int(data.warpWaves.y + 0.5));
    float4 pos = float4(p.x, fv.y, p.y, 1);
    vsOut.pos = float4(pos.x, 0, pos.z, 0);
    vsOut.ndcPos = mul(data.proj, mul(data.view, mul(data.model, pos)));
    return vsOut;
}