// Radix Sort
// Generate testing sequence

#include "sort.common.hlsl"
#include "../common/random.hlsl"

RWStructuredBuffer<uint> gSrc: register(u0,space0); // Input sequence

[numthreads(256,1,1)]
void main(uint3 globalInvo: SV_DispatchThreadID){
    float randomNum = trivialRandom(float2(globalInvo.x,frac(sin(float(globalInvo.x)))));
    gSrc[globalInvo.x] = uint(randomNum*1000)%4;
}
