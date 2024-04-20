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
static const float GRID_SIZE = 512.0;

float rand(float3 co)
{
    return frac(sin(dot(co.xyz, float3(12.9898, 78.233, 45.543))) * 43758.5453);
}

float fractalApproxPerlin(float3 co, float scaler, int octaves)
{
    float value = 0;
    float amplitude = 1;
    float frequency = 1;
    float sumAmpl = 0;
    for (int i = 0; i < octaves; i++)
    {
        value += cnoise(co * frequency* scaler) * amplitude;
        sumAmpl += amplitude;
        amplitude *= 0.75;
        frequency *= 2;
    }
    return value/sumAmpl;

}

[numthreads(8, 8, 8)]
void main(uint3 invId : SV_DispatchThreadID)
{
    float ampl = 1.5;
    float elev = 7.0;
    float3 pos = float3(invId) / GRID_SIZE + chunkLoc.loc.xyz + float3(4, 0, 4);
    float scaler = 2.99;
    float baseHeight = exp(2 * cnoise(float3(pos.xz * 4.5, 0))) * 0.2;
    float ret = (pos.y - baseHeight) * elev + 0.1;
    //ret += (exp(cnoise(pos * 1.95)) - 0.95) * ampl * 1.25;
    ret += fractalApproxPerlin(pos, scaler, 5) * ampl * sqrt(baseHeight);
    density[invId] = ret;
}