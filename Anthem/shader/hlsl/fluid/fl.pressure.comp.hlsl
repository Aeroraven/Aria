#include "fl.common.hlsl"

ConstantBuffer<FluidParams> fparams:register(b0,space0);
RWTexture2D<float4> baseVelocity:register(u0,space1);
RWTexture2D<float4> lastPressure:register(u0,space2);
RWTexture2D<float4> newPressure:register(u0,space3);

[numthreads(16,16,1)]
void main(uint3 invId:SV_DispatchThreadID){
    // Pressure
    // laplacian(p) = div(u)
    // (xL+xR+xT+xB-4xC) = div(u)
    // (xL+xR+xT+xB) - div(u) = 4xC
    // xC = (xL+xR+xT+xB - div(u)) * 0.25
    if(invId.x == 0 || invId.x == fparams.grid.x-1 || invId.y == 0 || invId.y == fparams.grid.y-1){
        float edgeOffsetX = 0;
        float edgeOffsetY = 0;
        if(invId.x == 0){
            edgeOffsetX = 1;
        }
        if(invId.y == 0){
            edgeOffsetY = 1;
        }
        if(invId.x == fparams.grid.x-1){
            edgeOffsetX = -1;
        }
        if(invId.y == fparams.grid.y-1){
            edgeOffsetY = -1;
        }
        newPressure[invId.xy] = lastPressure[invId.xy + int2(edgeOffsetX, edgeOffsetY)];
        return;
    }
    float alpha = -1.0;
    float rbeta = 0.25;
    jacobiIterDivergence(lastPressure, baseVelocity, invId.xy,
        alpha, rbeta, fparams.grid.xy, newPressure[invId.xy].x);
}