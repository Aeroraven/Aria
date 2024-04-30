#include "fl.common.hlsl"

ConstantBuffer<FluidParams> fparams:register(b0,space0);
RWTexture2D<float4> baseVelocity:register(u0,space1);
RWTexture2D<float4> lastVelocity:register(u0,space2);
RWTexture2D<float4> newVelocity:register(u0,space3);

[numthreads(16,16,1)]
void main(uint3 invId:SV_DispatchThreadID){
    // Diffusion
    // (I-v*dt*laplacian)u(x,t+dt) = u(x,t)
    // Center-term: 1+v*dt*4
    // Adjacency-term: -v*dt 
    // -v*dt(xL+xR+xT+xB)+(1+v*dt*4)xC=bX
    // (xL+xR+xT+xB)+bX/(v*dt)=(1+v*dt*4)/(v*dt)*xC
    if(invId.x == 0 || invId.x == fparams.grid.x-1 || invId.y == 0 || invId.y == fparams.grid.y-1){
        float offsetX = 0;
        float offsetY = 0;
        if(invId.x == 0){
            offsetX = 1;
        }
        if(invId.y == 0){
            offsetY = 1;
        }
        if(invId.x == fparams.grid.x-1){
            offsetX = -1;
        }
        if(invId.y == fparams.grid.y-1){
            offsetY = -1;
        }
        newVelocity[invId.xy] = -lastVelocity[invId.xy + int2(offsetX, offsetY)];
        return;
    }

    float coef = fparams.params.x;
    float dt = fparams.params.y;
    float alpha = 1.0/(coef*dt);
    float rbeta = 1/(4.0+alpha);

    float4 diffused;
    jacobiIter(lastVelocity, baseVelocity, invId.xy, alpha, rbeta, diffused);
    newVelocity[invId.xy] = diffused;
}