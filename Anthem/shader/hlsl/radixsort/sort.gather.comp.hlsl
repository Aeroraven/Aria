// Radix Sort
// Prefix sum for block sum

RWStructuredBuffer<uint> gBins: register(u0, space0);
ConstantBuffer<Uniforms> attrib: register(b0, space2);

static const uint BITS_PER_PASS = 4;
static const uint MAX_WORKGROUP_SIZE = 1024;

void main(uint GlobalInvo: SV_DispatchThreadID, uint3 WorkGroupId:SV_GroupID, uint LocalInvo:SV_GroupThreadID){
    
    // Step3. Prefix sum for block sum
    uint elemId = LocalInvo.x;
    uint blockId = WorkGroupId.x;
    uint globalId = GlobalInvo.x;
    uint seqLen = attrib.sequenceLength.x;
    uint numBlocks = seqLen/attrib.workGroupSize.x;

    uint totLen = (1<<BITS_PER_PASS)*numBlocks;
    uint logTotLen = uint(log2(totLen));

    // Reduce
    for(uint i = 0; i < logTotLen; i++){
        if((elemId & (1<<i)) == (1<<i)){
            gBins[elemId] += gBins[elemId-(1<<i)];
        }
        AllMemoryBarrierWithGroupSync();
    }

    // Scan
    for(uint i = logTotLen-2; i >= 0; i--){
        uint base = 1<<(i+1)-1;
        bool criteria = ((elemId-base) & (1<<(i+1)) == (1<<(i+1)));
        if(criteria){
            gBins[elemId] += gBins[elemId-(1<<i)];
        }
        AllMemoryBarrierWithGroupSync();
    }

}