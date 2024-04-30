#include "fl.common.hlsl"

ConstantBuffer<FluidParams> fparams:register(b0,space0);
RWTexture2D<float4> curVelocity:register(u0,space1);
RWTexture2D<float4> curPressure:register(u0,space2);
RWTexture2D<float4> outVelocity:register(u0,space3);

float2 getPressure(uint2 pos){
    if(pos.x <= 0 || pos.x >= fparams.grid.x-1 || pos.y <= 0 || pos.y >= fparams.grid.y-1){
        float offsetX = 0;
        float offsetY = 0;
        if(pos.x <= 0){
            offsetX = 1;
        }
        if(pos.y <= 0){
            offsetY = 1;
        }
        if(pos.x >= fparams.grid.x-1){
            offsetX = -1;
        }
        if(pos.y >= fparams.grid.y-1){
            offsetY = -1;
        }
        return curPressure[pos + uint2(offsetX, offsetY)].xy;
    }
    return curPressure[pos].xy;
}

[numthreads(16,16,1)]
void main(uint3 invId:SV_DispatchThreadID){
    // Subtract

    // Graidents of pressure
    float gradX  = 0;
    float gradY  = 0;
    if(invId.x > 0 && invId.x < fparams.grid.x-1){
        gradX = (getPressure(uint2(invId.x+1, invId.y)).x - getPressure(uint2(invId.x-1, invId.y)).x) * 0.5;
    }
    if(invId.y > 0 && invId.y < fparams.grid.y-1){
        gradY = (getPressure(uint2(invId.x, invId.y+1)).x - getPressure(uint2(invId.x, invId.y-1)).x) * 0.5;
    }
    float2 grad = float2(gradX, gradY);
    outVelocity[invId.xy] = curVelocity[invId.xy] - float4(grad, 0, 0);
}