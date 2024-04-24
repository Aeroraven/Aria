// Terrain Density Function
// Ref: https://developer.nvidia.com/gpugems/gpugems3/part-i-geometry/chapter-1-generating-complex-procedural-terrains-using-gpu

#include "pt.noise.hlsl"
#include "pt.common.hlsl"

struct ChunkLocation
{
    float4 loc; //(locl,locz,lod)
};

struct TreeLoc
{
    float4 treeLoc;
};

[[vk::push_constant]] ChunkLocation chunkLoc;
RWTexture3D<float> density : register(u0, space0);
AppendStructuredBuffer<TreeLoc> treeLocs : register(u0, space1);
static const float TERRAIN_ELEVATION = 0.5;
static const float OFFSET_X = 323.34;
static const float OFFSET_Z = 497.09;

float rand(float3 co)
{
    return frac(sin(dot(co.xyz, float3(12.9898, 78.233, 45.543))) * 43758.5453);
}

float sigmoidShift(float v, float shift)
{
    return 1.0 / (1.0 + exp(-v + shift));
}
float sigmoidMag(float v, float mag)
{
    return 1.0 / (1.0 + exp(-v*mag));
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
    float baseHeight = cnoise(dpos * frequency);
    float smoothTerm = (1 - smoothstep(0.35, 0.43, baseHeight));

    for (int i = 0; i < 5; i++)
    {
        value += cnoise(dpos * frequency) * amplitude;
        //Domain Warp
        dpos += cnoise(dpos * frequency) * (0.5 - 0.25 * smoothTerm) * sqrt(amplitude);
        sumAmpl += amplitude;
        amplitude *= 0.35 - 0.2 * smoothTerm;
        frequency *= 2.3 ;

    }
    float retv = pow(expElevation(value / sumAmpl, 0.2), 1.4);
    
    // Smooth for plain area
    retv = retv * sigmoidMag(retv-0.85, 2.3);
    return retv * 0.6 + 0.1;
}

float edgeModify(float2 v)
{
    float2 center = float2(0.5, 0.5) + float2(OFFSET_X, OFFSET_Z);
    float distCenter = length(center - v);
    float bv = smoothstep(0, 1, (1 - distCenter / length(float2(1.8, 1.8))));
    float modf = sigmoidShift((bv - 0.5) * 12.0, 0);
    if (bv < 0.5)
    {
        modf = exp((bv - 0.5) * 2.0)*0.5;
    }
    return 1-modf;
}
float centerElevation(float2 v)
{
    float2 center = float2(0.5, 0.5) + float2(OFFSET_X, OFFSET_Z);
    float distCenter = length(center - v);
    float modf = sigmoidShift(((distCenter / length(float2(1.5, 1.5))) - 0.5) * 8.0, 0);
    return modf ;
}

[numthreads(8, 8, 8)]
void main(uint3 invId : SV_DispatchThreadID)
{
    float3 unitSize = float3(GRID_SIZE / chunkLoc.loc.z - 1, GRID_SIZE_Y / chunkLoc.loc.z - 1, GRID_SIZE / chunkLoc.loc.z - 1) ;
    float ampl = 0.25;
    float elev = 14.0;
    float3 pos = float3(invId) / unitSize + float3(chunkLoc.loc.x, 0, chunkLoc.loc.y) + float3(OFFSET_X, 0, OFFSET_Z);
    float scaler = 1.4;
    float baseScaler = 0.30;
    
    // Base Height
    float baseHeight = fractalBaseHeightF(pos.xz * baseScaler);
    baseHeight = (baseHeight*0.9 + centerElevation(pos.xz) * 0.1) * edgeModify(pos.xz);
    //baseHeight = 0.3 * edgeModify(pos.xz);
    // Details
    float ret = 0;
    ret += (pos.y - baseHeight) * elev;
    ret += fractalApproxPerlin(pos, scaler, 2)*ampl;
    density[invId] = ret;
    
    float3 worldPos = (float3(invId)) / unitSize + float3(chunkLoc.loc.x, 0, chunkLoc.loc.y);
    worldPos = (worldPos - float3(0.5, 0, 0)) * float3(COORDINATE_SCALE, Y_ELEVATION, COORDINATE_SCALE);
    
    //Tree
    float delta = 1.0 / (GRID_SIZE_Y - 1.0);
    float noise = rand(worldPos.xzy);
    
    bool cond0 = baseHeight > 0.44 && baseHeight <= 0.50 && noise > 0.999;
    bool cond1 = baseHeight > 0.25 && baseHeight <= 0.44 && noise > 0.96;
    bool cond2 = baseHeight > 0.16 && baseHeight <= 0.25 && noise > 0.98;
    bool cond3 = baseHeight > 0.10 && baseHeight <= 0.16 && noise > 0.9987;

    if (pos.y > baseHeight && pos.y - delta < baseHeight && (cond0||cond1||cond3||cond2))
    {
        TreeLoc tloc;
        tloc.treeLoc = float4(worldPos, 1) ;
        treeLocs.Append(tloc);
    }
}