struct Attributes{
    float2 bary;
};
struct Payload{
    [[vk::location(0)]] float3 hitv;
};

Texture2D texModel[4] : register(t0, space3);
SamplerState sampModel[4] : register(s0, space3);

StructuredBuffer<float4> sbPosition : register(t0, space4);
StructuredBuffer<float4> sbNormal : register(t0, space5);
StructuredBuffer<float2> sbTexCoord : register(t0, space6);
StructuredBuffer<uint> sbIndices : register(t0, space7);
StructuredBuffer<uint> sbOffset : register(t0, space8);

void unpackTriangle(uint primitive, uint instance,inout uint ids[3]){
    const int totalInsts = 4;
    uint indOffset = sbOffset[instance + totalInsts];
    uint priOffset = sbOffset[instance];
    uint indice[3];
    for (uint i = 0; i < 3; i++){
        indice[i] = sbIndices[indOffset + primitive * 3 + i];
        ids[i] = priOffset + indice[i];
    }
}

float3 baryNormal(float2 uv,uint primitive,uint instance){
    uint ids[3];
    float3 ret = float3(0.0, 0.0, 0.0);
    float weights[3] = { 1.0 - uv.x - uv.y, uv.x, uv.y };
    unpackTriangle(primitive, instance, ids);
    for (uint i = 0; i < 3; i++)
        ret += weights[i] * sbNormal[ids[i]].xyz;
    ret = normalize(ret);
    return ret;
}

float2 baryTexUV(float2 uv, uint primitive, uint instance){
    uint ids[3];
    float2 ret = float2(0.0, 0.0);
    float weights[3] = { 1.0 - uv.x - uv.y, uv.x, uv.y };
    unpackTriangle(primitive, instance, ids);
    for (uint i = 0; i < 3; i++)
        ret += weights[i] * sbTexCoord[ids[i]].xy;
    return ret;
}

[shader("closesthit")]
void main(inout Payload p, in Attributes at){
    uint primId = PrimitiveIndex();
    uint instId = InstanceID();
    
    float3 norm = baryNormal(at.bary, primId, instId);
    float2 texUV = baryTexUV(at.bary, primId, instId);
    float4 texColor = texModel[instId].SampleLevel(sampModel[instId], texUV, 0);
    float4 color = texColor;
    
    p.hitv = color.rgb;
}
