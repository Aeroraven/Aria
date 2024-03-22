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
    float totalLights;
    float totalLightAreas;
    float time;
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
StructuredBuffer<uint> sbLightIndices : register(t0, space10);

static const float PI = 3.1415926;
static const float PI2 = 6.28318530717;
static const int TOTAL_INSTANCES = 17;
static const float IMP_SAMP_WEIGHT = 0.5;
static const float EPS = 1e-3;

static float debugInst = 0;
static float debugLightId = 0;
static float debugWarn = 0;
static float3 debugWarn3 = float3(0, 0, 0);
static float sd = 0;
// Random
float random(float2 seeds){
    //Random function from https://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl;
    return frac(sin(dot(seeds, float2(12.9898, 4.1414))) * 43758.5453);
    
    float a = 12.9898;
    float b = 78.233;
    float c = 43758.5453;
    float dt = dot(seeds, float2(a, b));
    float sn = fmod(dt, PI);
    return frac(sin(sn) * c);
}

float3 randomUnitVectorLegacy(float2 seeds1, float2 seeds2){
    float up = random(seeds2) * 2.0 - 1.0;
    float over = sqrt(max(0.0, 1.0 - up * up));
    float around = random(seeds1) * PI2;
    float x = cos(around) * over;
    float y = up;
    float z = sin(around) * over;
    return normalize(float3(z, x,y ));
}

float3 randomUnitVector(float2 seed1, float2 seed2)
{
    float3 randVec = float3(random(seed1), random(seed2 + 1.0), random(seed1 + seed2 + 2.0));
    return normalize(randVec * 2.0 - 1.0);
}

float3 randomHemisphereCosine(float2 seeds1, float2 seeds2){
    float rn1 = random(seeds1);
    float rn2 = random(seeds2);
    
    float ang = PI2 * rn1;
    float z = sqrt(1 - rn2);
    float x = sqrt(rn2) * cos(ang);
    float y = sqrt(rn2) * sin(ang);
    return float3(x, y, z);
}

bool rayTriangleIntersection(float3 origin, float3 dir, float3 v1, float3 v2, float3 v3,inout float lenx)
{ 
    float3 e1 = v2 - v1;
    float3 e2 = v3 - v1;
    float3 t = origin - v1;
    float3 dxe2 = cross(dir, e2);
    
    float det = dot(dxe2, e1);
    
    if (abs(det) < EPS)
        return false;

    
    float u = dot(dxe2, t) / det;
    if (u < -EPS || u > 1+EPS)
        return false;
    
    float3 txe1 = cross(t, e1);
    float v = dot(txe1, dir) / det;
    if (v < -EPS || u + v > 1+EPS)
        return false;

    float tp = dot(txe1, e2) / det;
    if(tp<=-EPS)
        return false;
    
    lenx = tp;
    return true;
}


// Buffer
void unpackTriangle(uint primitive, uint instance, inout uint ids[3]){
    uint indOffset = sbOffset[instance + TOTAL_INSTANCES];
    uint priOffset = sbOffset[instance];
    uint indice[3];
    for (uint i = 0; i < 3; i++){
        indice[i] = sbIndices[indOffset + primitive * 3 + i];
        ids[i] = priOffset + indice[i];
    }
}

float3 baryNormal(float2 uv, uint primitive, uint instance){
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

bool isChosenLight(uint primitive, uint instance)
{
    uint ids[3];
    unpackTriangle(primitive, instance, ids);
    uint lightInstance = sbLightIndices[debugLightId * 4 + 3];
    uint vertexOffset = sbOffset[lightInstance];
    bool flag = true;
    for (uint i = 0; i < 3; i++)
    {
        if (vertexOffset + sbLightIndices[debugLightId * 4 + i] != ids[i])
            flag = false;
    }
    return flag;

}
// Lambertian Reflection
float3 lambertOutRay(float3 normal, float seed1, float seed2){
    float3 randomVec = randomHemisphereCosine(float2(seed1, seed2), float2(seed2, seed1));
    float3 a = randomUnitVector(float2(sin(seed1) + seed2, sin(seed2) + seed1), float2(seed2, seed1));
    for (uint i = 0; i < 2; i++)
    {
        debugWarn = 0;
        seed2 += random(seed1);
        seed1 += random(seed2);
        a = randomUnitVector(float2(sin(seed1) + seed2, sin(seed2) + seed1), float2(seed2, seed1));
        if (length(a - normal) < EPS || length(a + normal) < EPS)
        {
            debugWarn = 1;
            continue;
        }
        break;
    }
        
    float3 t = normalize(cross(normal, a));
    float3 s = normalize(cross(normal, t));
    
    debugWarn3 = t * randomVec.x + s * randomVec.y + normal * randomVec.z;
    return t * randomVec.x + s * randomVec.y + normal * randomVec.z;
}

float lambertPdf(float3 normal, float3 outDir){
    return max(0, abs(dot(outDir, normal)) / PI);
}

// Importance Sampling
float3 importanceOutRay(float3 hitPoint, uint lightIdx, float prevTri1, float prevTri2, inout float3 targetSamplePt){
    uint lightInstance = sbLightIndices[lightIdx * 4 + 3];
    uint vertexOffset = sbOffset[lightInstance];
    debugInst = lightInstance;
    
    float3 v[3];
    for (uint i = 0; i < 3; i++){
        v[i] = sbPosition[vertexOffset + sbLightIndices[lightIdx * 4 + i]].xyz;
    }
    
    float r1 = random(float2(prevTri1, prevTri2));
    float r2 = random(float2(prevTri2, prevTri1));
    float3 samplePt = (1 - sqrt(r1)) * v[0] + sqrt(r1) * r2 * v[1] + sqrt(r1) * (1 - r2) * v[2];
    
    targetSamplePt = samplePt;
    return normalize(samplePt - hitPoint);
}

float importancePdf(float3 hitPoint,float3 normal, float3 outDir, uint lightIdx){
    uint lightInstance = sbLightIndices[lightIdx * 4 + 3];
    uint vertexOffset = sbOffset[lightInstance];
    float3 v[3];
    for (uint i = 0; i < 3; i++){
        v[i] = sbPosition[vertexOffset + sbLightIndices[lightIdx * 4 + i]].xyz;
    }
    float3 normalLight = normalize(cross(v[2] - v[0], v[1] - v[0]));
    float3 distVec = outDir;
    float w;
    if (rayTriangleIntersection(hitPoint, outDir, v[0], v[1], v[2],w) == false){
        return 0;
    }
    
    float3 e1 = v[1] - v[0];
    float3 e2 = v[2] - v[0];
    float area = length(cross(e1, e2)) / 2;
    
    float dist = w;
    distVec = normalize(distVec);
    float cosAngle = abs(dot(normalLight, distVec));
    return dist * dist / cosAngle / area;
}

// SRGB
float srgbToLinearSingle(float x){
    if (x < 0.0405)
        return x / 12.92;
    return pow((x + 0.055) / 1.055, 2.4);
}
float3 srgbToLinear(float3 x){
    return float3(srgbToLinearSingle(x.x), srgbToLinearSingle(x.y), srgbToLinearSingle(x.z));
}

// Scatter
float scatterPdf(float3 normal, float3 outDir){
    return max(0, abs(dot(outDir, normal)) / PI);
}

float3 scatter(float3 hitPoint, float3 hitnorm, uint impSampId, float rndSamp, float rnd1, float rnd2, inout float weight, inout float impSample){
    float3 nextRay;
    float sampCriteria = random(float2(rndSamp+rnd1, rndSamp+rnd2));
    if (sampCriteria <= 1 - IMP_SAMP_WEIGHT * impSample){
        nextRay = lambertOutRay(hitnorm, rnd1, rnd2);
    }
    else{
        //Importance Sampling
        float3 tgt;
        nextRay = importanceOutRay(hitPoint, impSampId, rnd1, rnd2, tgt);
        debugWarn3 = nextRay;

    }

    float pdf = lambertPdf(hitnorm, nextRay) * (1 - IMP_SAMP_WEIGHT * impSample);
    float impPdf = 0;
    for (uint i = 0; i < uint(cnt.totalLights+0.1); i++)
    {
        impPdf += importancePdf(hitPoint, hitnorm, nextRay, i) / cnt.totalLights;
    }
    pdf += IMP_SAMP_WEIGHT * impSample * impPdf;

    weight = scatterPdf(hitnorm, nextRay) / (EPS+pdf);
    if (pdf < EPS)
    {
        weight = 0;
    }

    return nextRay;
}


// Main Procedure
[shader("closesthit")]
void main(inout Payload p, in Attributes at){    
    // Unpack Buffers
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
    color = float4((texColor.xyz), 1.0);
    //color = float4(0.5, 0.5, 0.5, 1.0);
    
    // Generate Next Ray
   
    float colorWeight = 0;
    float drX = frac(cos(52.1 * p.iter)) + WorldRayOrigin().y + frac(cos(9.12 * pow(cnt.time, 1.1) + float(RayTCurrent())) * 0.17) + frac(sin(3.737 * cnt.counter)) - frac(sin(float(primId)));
    float drY = frac(sin(15.4 * p.iter)) + WorldRayOrigin().x + frac(cos(4.89 * sqrt(cnt.time) + float(RayTCurrent())) * 0.29) + frac(cos(3.737 * cnt.counter)) - frac(PI * cos(float(primId)));
    float srd = random(float2(drX,drY));
    uint lightIdx = uint(srd * cnt.totalLights);
    
    float rndX = frac(sin(2.1 * p.innerIter)) + WorldRayOrigin().y + frac(cos(4.12 * cnt.time + float(RayTCurrent())) * 745.17) + frac(sin(6.77 * cnt.counter)) - frac(cos(float(primId)));
    float rndY = frac(sin(7.9 * p.innerIter)) + WorldRayOrigin().x + frac(cos(1.27 * pow(cnt.time, 1.1) + float(RayTCurrent())) * 145.17) + frac(sin(6.77 * sqrt(cnt.counter))) - frac(cos(float(primId)));
    float rndW = frac(sin(1.4 * p.innerIter)) + WorldRayOrigin().x + WorldRayOrigin().y + frac(sin(cnt.time) * 1149.14) + WorldRayDirection().y * WorldRayDirection().x;
    float impSample = 1.0;
    if (InstanceID() == 3){
        impSample = 0.0;
    }
    float3 nextRayDir = scatter(WorldRayDirection() * RayTCurrent() + WorldRayOrigin(), norm, lightIdx, rndW, rndX, rndY, colorWeight, impSample);
    
    Payload nextPayload;
    float3 nextDir = nextRayDir;
    float3 nextOrigin = WorldRayDirection() * RayTCurrent() + WorldRayOrigin();
    
    nextPayload.hitv = float3(0.0, 0.0, 0.0);
    nextPayload.innerIter = p.innerIter;
    
    const float lumin = 40.0;
    if (InstanceID() == 3){
        p.hitv = float3(lumin, lumin, lumin);
    }

    RayDesc newRay;
    newRay.Direction = nextDir;
    newRay.Origin = nextOrigin;
    newRay.TMax = 10000.0;
    newRay.TMin = 0.01;
    
    if (p.iter <= 5)
    {
        nextPayload.iter = p.iter + 1;
        TraceRay(accStruct, RAY_FLAG_FORCE_OPAQUE, 0xff, 0, 0, 0, newRay, nextPayload);
    }
    p.hitv += color.rgb * nextPayload.hitv * colorWeight;
    //p.hitv = abs(norm.xyz);

}
