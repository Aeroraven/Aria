// Radix Sort
// Scatter and Sort

#include "sort.common.hlsl"

RWStructuredBuffer<uint> gSrc: register(u0,space0); // Input sequence
RWStructuredBuffer<uint> gDst: register(u0,space1); // Output sequence
RWStructuredBuffer<uint> gBins: register(u0,space2); // Block sums
RWStructuredBuffer<uint> gLocalPrefixSum: register(u0,space3); // Prefix Sum
ConstantBuffer<Uniforms> attrib: register(b0,space4);

static const uint BITS_PER_PASS = 2;
groupshared uint maxval = 0;

struct PushConstant{
    uint4 CURRENT_BIT;
};
[[vk::push_constant]] PushConstant pc;


[numthreads(1024,1,1)]
void main(uint GlobalInvo: SV_DispatchThreadID, uint3 WorkGroupId:SV_GroupID, uint LocalInvo:SV_GroupThreadID){
    //Step4. Scatter
    
    uint globalId = GlobalInvo.x;
    uint blockSize = attrib.workGroupSize.x;
    uint numBlocks = attrib.sequenceLength.x/blockSize;
    uint blockId = globalId/blockSize;

    uint localPrefixSum = gLocalPrefixSum[globalId];
    uint curNum = gSrc[globalId];
    uint curBit = (curNum>>(pc.CURRENT_BIT.x*BITS_PER_PASS)) & ((uint(1)<<BITS_PER_PASS)-1);
    uint blockPrefixSum = gBins[curBit*numBlocks+blockId];

    uint scatterPos = blockPrefixSum + localPrefixSum;
    gDst[scatterPos] = gSrc[globalId];
    //gDst[globalId] = blockPrefixSum;
}