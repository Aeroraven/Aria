#include "fl.common.hlsl"

ConstantBuffer<FluidParams> fparams:register(b0,space0);
RWTexture2D<float4> curVelocity:register(u0,space1);
RWTexture2D<float4> curPressure:register(u0,space2);
RWTexture2D<float4> outVelocity:register(u0,space3);

[numthreads(16,16,1)]
void main(uint3 invId:SV_DispatchThreadID){
    // Subtract
    outVelocity[invId.xy] = curVelocity[invId.xy] - curPressure[invId.xy];
}