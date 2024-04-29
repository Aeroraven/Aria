#include "fl.common.hlsl"

ConstantBuffer<FluidParams> fparams:register(b0,space0);
RWTexture2D<float4> lastVelocity:register(u0,space1);
RWTexture2D<float4> qtyToAdvect:register(u0,space2);
RWTexture2D<float4> qtyResult:register(u0,space3);

[numthreads(16,16,1)]
void main(uint3 invId:SV_DispatchThreadID){
    // Advection
    // Q[x,t+dt] = Q[x-u[x,t]dt,t]

    float2 u = lastVelocity[invId.xy].xy;
    float2 uv = invId.xy - u * fparams.params.y;
    float4 q = qtyToAdvect[uint2(uv)];
    qtyResult[invId.xy] = q;
}