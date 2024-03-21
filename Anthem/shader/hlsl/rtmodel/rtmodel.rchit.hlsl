struct Attributes{
    float2 bary;
};
struct Payload{
    [[vk::location(0)]] float3 hitv;
    [[vk::location(1)]] float shadow;
};

RaytracingAccelerationStructure accStruct : register(t0, space0);
struct Camera
{
    float4x4 projInv;
    float4x4 viewInv;
};

ConstantBuffer<Camera> cam : register(b0, space2);

Texture2D texModel[4] : register(t0, space3);
SamplerState sampModel[4] : register(s0, space3);

StructuredBuffer<float4> sbPosition : register(t0, space4);
StructuredBuffer<float4> sbNormal : register(t0, space5);
StructuredBuffer<float2> sbTexCoord : register(t0, space6);
StructuredBuffer<uint> sbIndices : register(t0, space7);
StructuredBuffer<uint> sbOffset : register(t0, space8);

void unpackTriangle(uint primitive, uint instance,inout uint ids[3]){
    const int totalInsts = 5;
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
    
    float4 color = float4(1.0, 1.0, 1.0, 1.0);
    float3 norm = baryNormal(at.bary, primId, instId);
    if (instId < 4)
    {
        
        float2 texUV = baryTexUV(at.bary, primId, instId);
        float4 texColor = texModel[instId].SampleLevel(sampModel[instId], texUV, 0);
        color = texColor;
    }
    
    p.shadow = 1.0;
    RayDesc ray;
    ray.Origin = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
    ray.Direction = normalize(float3(1, 1, 1));
    ray.TMin = 0.002;
    ray.TMax = 10000.0;

    TraceRay(accStruct, 
        RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER,
        0xff, 0, 0, 1, ray, p);
    
    p.hitv = color.rgb;
    float diffuse = max(0.0, dot(normalize(float3(1, 1, 1)), norm));
    p.hitv = p.hitv * (0.2 + diffuse * (1.0 - p.shadow));
}
