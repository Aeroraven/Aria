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
    float coef = fparams.params.x;
    float dt = fparams.params.y;
    float alpha = 1.0/(coef*dt);
    float rbeta = (coef*dt)/(1.0+coef*dt*4.0);

    float4 diffused;
    jacobiIter(lastVelocity, baseVelocity, invId.xy, alpha, rbeta, diffused);
    newVelocity[invId.xy] = diffused;
}