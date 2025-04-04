#include "fl.common.hlsl"

ConstantBuffer<FluidParams> fparams:register(b0,space0);
RWTexture2D<float4> outVelocity:register(u0,space1);
RWTexture2D<float4> outDye:register(u0,space2);


float3 hsv2rgb(float3 c)
{
    // Reference: http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
    float4 K = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    float3 p = abs(frac(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * lerp(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}


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
    outVelocity[invId.xy].w=1;
    outDye[invId.xy] = outDye[invId.xy] + float4(0.3+hsv2rgb(float3(frac(fparams.dye.y),1,1)),0)*power;
    outDye[invId.xy].w=fparams.dye.x;
}