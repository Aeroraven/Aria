// Compute Frustum Culling
#include "pt.common.hlsl"
struct TreeLoc
{
    float4 treeLoc;
};
struct TreeLocCounter
{
    int counter;
};
RWStructuredBuffer<TreeLoc> treeLocs : register(u0, space0);
RWStructuredBuffer<TreeLocCounter> treeLocCounter : register(u1, space0);
AppendStructuredBuffer<TreeLoc> outLocs : register(u0, space1);
ConstantBuffer<Camera> cam :register(b0, space2);

[numthreads(64, 1, 1)]
void main(uint3 invIdx : SV_DispatchThreadID)
{
    uint3 idx = invIdx;
    if (idx.x >= treeLocCounter[0].counter)
    {
        return;
    }
    TreeLoc treeLoc = treeLocs[idx.x];
    float4 worldPos = float4(treeLoc.treeLoc.xyz, 1.0f);
    float4 clipPos = mul(cam.proj, mul(cam.view, worldPos));
    clipPos /= clipPos.w;
    if (clipPos.x < -1 || clipPos.x > 1 || clipPos.y < -1 || clipPos.y > 1 || clipPos.z < 0 || clipPos.z > 1)
    {
        return;
    }
    outLocs.Append(treeLoc);
}