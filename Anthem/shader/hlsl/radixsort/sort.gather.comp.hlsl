// Radix Sort
// Prefix sum for block sum
#include "sort.common.hlsl"
RWStructuredBuffer<uint> gBins: register(u0, space0);
ConstantBuffer<Uniforms> attrib: register(b0, space1);

static const uint BITS_PER_PASS = 2;
static const uint MAX_WORKGROUP_SIZE = 256;

groupshared uint binSrcValues[256*(uint(1)<<BITS_PER_PASS)];

struct PushConstant{
    uint4 CURRENT_BIT;
};
[[vk::push_constant]] PushConstant pc;

[numthreads(256*(uint(1)<<BITS_PER_PASS),1,1)]
void main(uint GlobalInvo: SV_DispatchThreadID, uint3 WorkGroupId:SV_GroupID, uint LocalInvo:SV_GroupThreadID){
    
    // Step3. Prefix sum for block sum
    uint elemId = LocalInvo.x;
    uint blockId = WorkGroupId.x;
    uint globalId = GlobalInvo.x;
    uint seqLen = attrib.sequenceLength.x;
    uint numBlocks = seqLen/attrib.workGroupSize.x;

    uint totLen = (int(1)<<BITS_PER_PASS)*numBlocks;
    uint logTotLen = uint(log2(totLen));

    // Record
    binSrcValues[elemId] = gBins[elemId];
    GroupMemoryBarrierWithGroupSync();

    // Reduce
    for(uint i = 0; i < logTotLen; i++){
        int base = (int(1)<<(i+1))-1;
        int interval = int(1)<<(i+1);

        if(elemId>=base&&(elemId-base)%interval==0){
            gBins[elemId] += gBins[elemId-(uint(1)<<i)];
        }
        DeviceMemoryBarrierWithGroupSync();
        
    }
    // Scan
    for(int i = logTotLen-2; i >= 0; i--){
        uint base = (uint(1)<<(i+1))-1;
        uint first = base+(int(1)<<i);
        bool criteria = (((elemId-first)%(int(1)<<(i+1)))==0) && (elemId>=first);

        if(criteria){
            gBins[elemId] += gBins[elemId-(uint(1)<<i)];
        }
        DeviceMemoryBarrierWithGroupSync();
    }

    gBins[elemId] -= binSrcValues[elemId];
}