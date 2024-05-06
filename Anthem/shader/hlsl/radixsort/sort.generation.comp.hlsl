// Radix Sort
// Generate testing sequence

#include "sort.common.hlsl"
#include "../common/random.hlsl"

RWStructuredBuffer<uint> gSrc: register(u0,space0); // Input sequence

[numthreads(1024,1,1)]
void main(uint3 globalInvo: SV_DispatchThreadID){
    float randomNum = trivialRandom(float2(globalInvo.x,frac(sin(float(globalInvo.x)))));
    float randomNum2 = sin(trivialRandom(float2(globalInvo.x,frac(sin(float(globalInvo.x)))))*3.1415926);
    gSrc[globalInvo.x] = uint(randomNum*16777215/2+randomNum2*16777215/2)%(16777215);
}
