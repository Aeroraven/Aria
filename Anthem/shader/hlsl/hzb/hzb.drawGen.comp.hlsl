struct OccludeePos{
	float4 pos;
};
struct IndirectBuffer{
    uint indexCount;
    uint instanceCount;
    uint firstIndex;
    uint vertexOffset;
    uint firstInstance;
};
struct DrawParams{
    uint indexCount;
};

RWStructuredBuffer<OccludeePos> occludeePos : register(u0, space0);
RWStructuredBuffer<uint> posCounter : register(u1, space0);
RWStructuredBuffer<IndirectBuffer> indirectBuffer : register(u0, space1);
[[vk::push_constant]] DrawParams params;

[numthreads(1, 1, 1)]
void main(uint3 invId:SV_DispatchThreadID){
    indirectBuffer[0].indexCount = params.indexCount;
    indirectBuffer[0].instanceCount = posCounter[0];
    indirectBuffer[0].firstIndex = 0;
    indirectBuffer[0].vertexOffset = 0;
    indirectBuffer[0].firstInstance = 0;
}