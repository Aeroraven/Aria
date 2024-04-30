#include "fl.common.hlsl"

ConstantBuffer<FluidParams> fparams:register(b0,space0);
RWTexture2D<float4> lastVelocity:register(u0,space1);
RWTexture2D<float4> qtyToAdvect:register(u0,space2);
RWTexture2D<float4> qtyResult:register(u0,space3);

float4 interpolatedQty(float2 uv){
    int2 uv0 = int2(uv);
    int2 uv1 = uv0 + int2(1,0);
    int2 uv2 = uv0 + int2(0,1);
    int2 uv3 = uv0 + int2(1,1);
    float2 f = uv - uv0;
    float4 q0 = qtyToAdvect[uv0];
    float4 q1 = qtyToAdvect[uv1];
    float4 q2 = qtyToAdvect[uv2];
    float4 q3 = qtyToAdvect[uv3];
    return lerp(lerp(q0, q1, f.x), lerp(q2, q3, f.x), f.y);

}

[numthreads(16,16,1)]
void main(uint3 invId:SV_DispatchThreadID){
    // Advection
    // Q[x,t+dt] = Q[x-u[x,t]dt,t]

    float2 u = lastVelocity[invId.xy].xy;
    float2 uv = float2(int2(invId.xy) - u * fparams.params.y);
    if(uv.x<0||uv.y>fparams.grid.x||uv.y<0||uv.y>fparams.grid.y){
        qtyResult[invId.xy] = float4(0,0,0,0);
        return;
    }
    float4 q = interpolatedQty(uv);
    qtyResult[invId.xy] = float4(q.xyz*q.w,q.w); //Decay
}