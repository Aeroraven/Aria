struct Attributes{
    float2 bary;
};
struct Payload{
    [[vk::location(0)]] float3 hitv;
    [[vk::location(1)]] uint iter;
    [[vk::location(2)]] uint innerIter;
};

RaytracingAccelerationStructure accStruct : register(t0, space0);
struct Camera{
    float4x4 projInv;
    float4x4 viewInv;
};

struct Counter{
    float counter;
};

ConstantBuffer<Camera> cam : register(b0, space2);
ConstantBuffer<Counter> cnt : register(b0, space9);

Texture2D texModel[17] : register(t0, space3);
SamplerState sampModel[17] : register(s0, space3);

StructuredBuffer<float4> sbPosition : register(t0, space4);
StructuredBuffer<float4> sbNormal : register(t0, space5);
StructuredBuffer<float2> sbTexCoord : register(t0, space6);
StructuredBuffer<uint> sbIndices : register(t0, space7);
StructuredBuffer<uint> sbOffset : register(t0, space8);

float random(float2 seeds){
    //Random function from https://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl;
    float a = 12.9898;
    float b = 78.233;
    float c = 43758.5453;
    float dt = dot(seeds * cnt.counter, float2(a, b));
    float sn = fmod(dt, 3.14);
    return frac(sin(sn) * c);
}

float3 randomUnitVector(float2 seeds1, float2 seeds2){
    float up = random(seeds1) * 2.0 - 1.0;
    float over = sqrt(max(0.0, 1.0 - up * up));
    float around = random(seeds2) * 6.28318530717;
    return normalize(float3(cos(around) * over, up, sin(around) * over));
}



void unpackTriangle(uint primitive, uint instance, inout uint ids[3]){
    const int totalInsts = 17;
    uint indOffset = sbOffset[instance + totalInsts];
    uint priOffset = sbOffset[instance];
    uint indice[3];
    for (uint i = 0; i < 3; i++){
        indice[i] = sbIndices[indOffset + primitive * 3 + i];
        ids[i] = priOffset + indice[i];
    }
}

float3 baryNormal(float2 uv, uint primitive, uint instance)
{
    uint ids[3];
    float3 ret = float3(0.0, 0.0, 0.0);
    float weights[3] = { 1.0 - uv.x - uv.y, uv.x, uv.y };
    unpackTriangle(primitive, instance, ids);
    for (uint i = 0; i < 3; i++)
        ret += weights[i] * sbNormal[ids[i]].xyz;
    ret = normalize(ret);
    return ret;
}

float2 baryTexUV(float2 uv, uint primitive, uint instance)
{
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
        
    float2 texUV = baryTexUV(at.bary, primId, instId);
    uint texId = instId;
    float4 texColor = texModel[texId].SampleLevel(sampModel[texId], texUV, 0);
    if (dot(WorldRayDirection(), norm) > 0){
        norm = -norm;
    }
    color = texColor; 

    
    Payload nextPayload;
    float3 nextDir = randomUnitVector(RayTCurrent()+cnt.counter*1.41, cnt.counter*7.41+float(p.innerIter)+float(PrimitiveIndex()) / 451.0);
    float3 nextOrigin = WorldRayDirection() * RayTCurrent() + WorldRayOrigin();
    
    float3 nextDir2 = normalize(norm + nextDir);
    nextPayload.hitv = float3(0.0, 0.0, 0.0);
    nextPayload.innerIter = p.innerIter;
    
    RayDesc newRay;
    newRay.Direction = nextDir2;
    newRay.Origin = nextOrigin;
    newRay.TMax = 10000.0;
    newRay.TMin = 0.01;
    
    if (p.iter <= 8){
        nextPayload.iter = p.iter + 1;
        TraceRay(accStruct, RAY_FLAG_FORCE_OPAQUE, 0xff, 0, 0, 0, newRay, nextPayload);
    }
    //Lambertian
    float cosv = max(0, dot(nextDir2, norm));
    p.hitv = color.rgb * nextPayload.hitv * cosv ;

    const float lumin = 150.0;
    if (InstanceID() == 3){
        p.hitv += float3(lumin, lumin, lumin);
    }

}
