#include "fl.common.hlsl"

ConstantBuffer<FluidParams> fparams:register(b0,space0);
RWTexture2D<float4> outDye:register(u0,space1);
RWTexture2D<float4> outVelocity:register(u0,space2);

[numthreads(16,16,1)]
void main(uint3 invId:SV_DispatchThreadID){
    // Initialize
    outDye[invId.xy] = float4(0.0, 0.0, 0.0, 1.0);
    outVelocity[invId.xy] = float4(0.0, 0.0, 0.0, 0.0);
    return;
}