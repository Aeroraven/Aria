
float recoverZDepth(float ndcz, float f, float n)
{
    return f * n / (f - ndcz * (f - n));
}
float4 invId2Ndc(uint3 invId, int3 volSize, float f,float n)
{
    float3 norm = (float3(invId) + 0.5) / float3(volSize);
    float zVal = lerp(n, f, norm.z);
    float2 ndcXY = (norm.xy * 2.0 - 1.0) * zVal;    
    float ndcZ = f * (zVal - n) / (f - n);
    return float4(ndcXY, ndcZ, zVal);
}
float4 invId2NdcJitter(uint3 invId, int3 volSize, float f, float n,float3 jitter)
{
    float3 norm = (float3(invId) + 0.5 + jitter) / float3(volSize);
    float zVal = lerp(n, f, norm.z);
    float2 ndcXY = (norm.xy * 2.0 - 1.0) * zVal;
    float ndcZ = f * (zVal - n) / (f - n);
    return float4(ndcXY, ndcZ, zVal);
}
float4 ndc2WorldPos(float4 ndc, float4x4 invVp)
{
    return mul(invVp, ndc);
}
float4 invId2WorldPos(uint3 invId, int3 volSize, float4x4 invVp, float f, float n)
{
    float4 ndc = invId2Ndc(invId, volSize, f, n);
    return ndc2WorldPos(ndc, invVp);
}
float4 invId2WorldPosJitter(uint3 invId, int3 volSize, float4x4 invVp, float f, float n, float3 jitter)
{
    float4 ndc = invId2NdcJitter(invId, volSize, f, n, jitter);
    return ndc2WorldPos(ndc, invVp);
}