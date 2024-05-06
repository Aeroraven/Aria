// Radix sort
// Intra-block information Gathering

#include "sort.common.hlsl"

RWStructuredBuffer<uint> gSrc: register(u0,space0); // Input sequence
RWStructuredBuffer<uint> gBins: register(u0,space1); // Block sums
RWStructuredBuffer<uint> gLocalPrefixSum: register(u0,space2); // Prefix Sum
ConstantBuffer<Uniforms> attrib: register(b0,space3); 

static const uint BITS_PER_PASS = 2;
static const uint MAX_WORKGROUP_SIZE = 1024;

groupshared uint prefixCounter[uint(1)<<BITS_PER_PASS][MAX_WORKGROUP_SIZE];
groupshared uint localBlockSum[uint(1)<<BITS_PER_PASS];

struct PushConstant{
    uint4 CURRENT_BIT;
};
[[vk::push_constant]] PushConstant pc;

[numthreads(MAX_WORKGROUP_SIZE,1,1)]
void main(uint GlobalInvo: SV_DispatchThreadID, uint3 WorkGroupId:SV_GroupID, uint LocalInvo:SV_GroupThreadID){
    uint elemId = LocalInvo.x;
    uint blockId = WorkGroupId.x;
    uint globalId = GlobalInvo.x;
    uint curIter = pc.CURRENT_BIT.x;
    uint logBlockSize = uint(log2(attrib.workGroupSize.x));
    uint bitCandidates = int(1)<<BITS_PER_PASS;
    uint seqLen = attrib.sequenceLength.x;
    uint numBlocks = seqLen/attrib.workGroupSize.x;

    uint elem = gSrc[globalId];
    uint elemBit = (elem >> (curIter*BITS_PER_PASS)) & ((uint(1)<<(BITS_PER_PASS))-1);

    if(elemId<bitCandidates)
        localBlockSum[elemId] = 0;
    GroupMemoryBarrierWithGroupSync();
    
    InterlockedAdd(localBlockSum[elemBit],1);

    //Step1. Calculate local prefix sum
    for(uint i = 0; i < bitCandidates; i++){
        prefixCounter[i][elemId] = (elemBit==i)?1:0;
    }
    GroupMemoryBarrierWithGroupSync();

    //Reduce
    for(uint i = 0; i < bitCandidates; i++){

        for(uint j = 0; j < logBlockSize;j++){
            // A[k]=A[k]+A[k-2^i] for k = 2^i+1, 2^i+3, ...
            int base = (int(1)<<(j+1))-1;
            int interval = int(1)<<(j+1);
            if(elemId>=base&&(elemId-base)%interval==0){
                prefixCounter[i][elemId] +=  prefixCounter[i][elemId-(uint(1)<<j)];
            }
            GroupMemoryBarrierWithGroupSync();
            
        }
 

        //Scan

        for(int j = logBlockSize-2; j >= 0; j--){
            uint base = (uint(1)<<(j+1))-1;
            uint first = base+(int(1)<<j);
            bool criteria = (((elemId-first)%(int(1)<<(j+1)))==0) && (elemId>=first);
            if(criteria){
                prefixCounter[i][elemId] += prefixCounter[i][elemId-(int(1)<<j)];
            }
            GroupMemoryBarrierWithGroupSync();
        }
    }
    gLocalPrefixSum[globalId] = prefixCounter[elemBit][elemId]-1;

    //Step2. Modify Global Block Sum
    if(elemId<bitCandidates){
        gBins[elemId*numBlocks+blockId] = localBlockSum[elemId];
    }
    AllMemoryBarrierWithGroupSync();
}