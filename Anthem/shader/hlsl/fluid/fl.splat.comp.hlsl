#include "fl.common.hlsl"

ConstantBuffer<FluidParams> fparams:register(b0,space0);
RWTexture2D<float4> outVelocity:register(u0,space1);
RWTexture2D<float4> outDye:register(u0,space2);

[numthreads(16, 16, 1)]
void main(uint3 invId:SV_DispatchThreadID){
    //Splat
    float2 dVelocity = fparams.splat.xy;
    float2 dSplatP = fparams.splat.zw/float2(fparams.window.xy)*float2(fparams.grid.xy);
    float dEnable = fparams.params.w;
    if(dEnable<0.5)return;

    float power = distance(dSplatP, float2(invId.xy));
    power = exp(-power*power/fparams.params.z);
    outVelocity[invId.xy] = outVelocity[invId.xy] + float4(dVelocity, 0, 0)*power;
    outDye[invId.xy] = outDye[invId.xy] + float4(1, 0, 0, 1)*power;
}