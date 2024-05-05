// Radix sort
// Intra-block information Gathering

#include "sort.common.hlsl"

RWStructuredBuffer<uint> gSrc: register(u0,space0); // Input sequence
RWStructuredBuffer<uint> gBins: register(u0,space1); // Block sums
RWStructuredBuffer<uint> gLocalPrefixSum: register(u0,space2); // Prefix Sum
ConstantBuffer<Uniforms> attrib: register(b0,space3); 

static const uint BITS_PER_PASS = 4;
static const uint MAX_WORKGROUP_SIZE = 256;

groupshared uint prefixCounter[1<<BITS_PER_PASS][MAX_WORKGROUP_SIZE];
groupshared uint localBlockSum[1<<BITS_PER_PASS];

[numthreads(MAX_WORKGROUP_SIZE,1,1)]
void main(uint GlobalInvo: SV_DispatchThreadID, uint3 WorkGroupId:SV_GroupID, uint LocalInvo:SV_GroupThreadID){
    uint elemId = LocalInvo.x;
    uint blockId = WorkGroupId.x;
    uint globalId = GlobalInvo.x;
    uint curIter = attrib.parallelBits.x;
    uint logBlockSize = uint(log2(attrib.workGroupSize.x));
    uint bitCandidates = 1<<BITS_PER_PASS;
    uint seqLen = attrib.sequenceLength.x;
    uint numBlocks = seqLen/attrib.workGroupSize.x;

    uint elem = gSrc[globalId];
    uint elemBit = (elem >> (curIter*BITS_PER_PASS)) & (1<<(BITS_PER_PASS)-1);
    InterlockedAdd(localBlockSum[elemBit],1);

    //Step1. Calculate local prefix sum
    prefixCounter[elemBit][elemId] = 1;
    GroupMemoryBarrierWithGroupSync();

    //Reduce
    for(uint i = 0; i < bitCandidates; i++){
        for(uint j = 0; j < logBlockSize;j++){
            // A[k]=A[k]+A[k-2^i] for k = 2^i+1, 2^i+3, ...
            if((elemId&(1<<j))==(1<<j)){
                prefixCounter[i][elemId] += prefixCounter[i][elemId-(1<<j)];
            }
            GroupMemoryBarrierWithGroupSync();
        }
    }

    //Scan
    for(uint i = 0; i < bitCandidates; i++){
        for(uint j = logBlockSize-2; j >= 0; j--){
            uint base = 1<<(j+1)-1;
            bool criteria = ((elem-base)&(1<<(j+1))==(1<<(j+1)));
            if(criteria){
                prefixCounter[i][elemId] += prefixCounter[i][elemId-(1<<j))];
            }
            GroupMemoryBarrierWithGroupSync();
        }
    }
    gLocalPrefixSum[globalId] = prefixCounter[elemBit][elemId]-1;

    //Step2. Modify Global Block Sum
    if(elemId<bitCandidates){
        InterlockedAdd(gBins[elemId*bitCandidates+blockId],localBlockSum[elemId]);
    }
    GroupMemoryBarrierWithGroupSync();
}