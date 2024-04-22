// Terrain Density Function
// Ref: https://developer.nvidia.com/gpugems/gpugems3/part-i-geometry/chapter-1-generating-complex-procedural-terrains-using-gpu

#include "pt.noise.hlsl"
struct ChunkLocation
{
    float4 loc;
};

[[vk::push_constant]] ChunkLocation chunkLoc;
RWTexture3D<float> density : register(u0, space0);
static const float TERRAIN_ELEVATION = 0.5;
static const float GRID_SIZE = 168.0;

float rand(float3 co)
{
    return frac(sin(dot(co.xyz, float3(12.9898, 78.233, 45.543))) * 43758.5453);
}



float sigmoidShift(float v, float shift)
{
    return 1.0 / (1.0 + exp(-v + shift));
}

float fractalApproxPerlin(float3 co, float scaler, int octaves)
{
    float value = 0;
    float amplitude = 1;
    float frequency = 1;
    float sumAmpl = 0;
    for (int i = 0; i < octaves; i++)
    {
        value += cnoise(co.xzy * frequency * scaler) * amplitude;
        sumAmpl += amplitude;
        amplitude *= 0.75;
        frequency *= 3;
    }
    return value/sumAmpl;

}
float expElevation(float v,float shift)
{
    return (exp(v + shift) - exp(shift));
}
float fractalBaseHeightF(float2 pos)
{
    float sumAmpl = 0;
    float value = 0;
    float amplitude = 1;
    float frequency = 1;
    float3 dpos = float3(pos, 0);
    for (int i = 0; i < 5; i++)
    {
        value += cnoise(dpos * frequency) * amplitude;
        //Domain Warp
        dpos += cnoise(dpos * frequency) * 0.45 * sqrt(amplitude);
        sumAmpl += amplitude;
        amplitude *= 0.35;
        frequency *= 2;
    }
    float retv = pow(expElevation(value / sumAmpl, 0.2), 1.4);
    
    // Smooth for plain area
    retv = retv * sigmoidShift(retv, 0.7);
    return retv * 0.65;
}

float edgeModify(float2 v)
{
    float2 center = float2(0.5, 0.5) + float2(228.382, 497.09);
    float distCenter = length(center - v);
    float bv = (1 - distCenter / length(float2(1.5, 1.5)));
    float modf = sigmoidShift((bv - 0.5) * 8.0, 0);
    if (bv < 0.5)
    {
        modf = exp((bv - 0.5) * 8.0)*0.5;
    }
    return modf;
}
float centerElevation(float2 v)
{
    float2 center = float2(0.5, 0.5) + float2(228.382, 497.09);
    float distCenter = length(center - v);
    float modf = sigmoidShift(((1 - distCenter / length(float2(1.5, 1.5))) - 0.5) * 8.0, 0);
    return modf;

}


[numthreads(8, 8, 8)]
void main(uint3 invId : SV_DispatchThreadID)
{
    float ampl = 0.05;
    float elev = 14.0;
    float3 pos = float3(invId) / (GRID_SIZE - 1) + float3(chunkLoc.loc.x, 0, chunkLoc.loc.y) + float3(228.382, 0, 497.09);
    float scaler = 15.5;
    float baseScaler = 1.05;
    
    // Base Height
    float baseHeight = fractalBaseHeightF(pos.xz * baseScaler);
    baseHeight = (baseHeight + centerElevation(pos.xz) * 0.3) * edgeModify(pos.xz);
    // Details
    float ret = 0;
    ret += (pos.y - baseHeight) * elev;
    ret -= fractalApproxPerlin(pos, scaler, 1)*ampl;
    density[invId] = ret;
}