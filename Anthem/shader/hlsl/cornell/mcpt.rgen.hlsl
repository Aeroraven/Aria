RaytracingAccelerationStructure accStruct : register(t0,space0);
RWTexture2D<float4> outImage : register(u0,space1);
struct Camera
{
    float4x4 projInv;
    float4x4 viewInv;
};
struct Counter
{
    float counter;
    float totalLights;
    float totalLightAreas;
    float time;
};

ConstantBuffer<Counter> cnt : register(b0, space8);
ConstantBuffer<Camera> cam : register(b0, space2);
struct Payload
{
    [[vk::location(0)]] float3 hitv;
    [[vk::location(1)]] uint iter;
    [[vk::location(2)]] uint innerIter;
    [[vk::location(3)]] bool discardRay;
};

float4 gammaCorrection(float4 base,bool inv)
{
    float b = 1.0 / 2.2;
    if (inv)
        b = 1 / b;
    return pow(base, float4(b, b, b, b));
}

float4 toneMapping(float4 base, bool inv)
{
    return base;
    if (!inv)
    {
        return base / (base + 1);
    }
    return base / (1 - base);
}
float random(float2 seeds)
{
    return frac(sin(dot(seeds, float2(12.9898, 4.1414))) * 43758.5453);
}
[shader("raygeneration")]
void main()
{
    uint3 lId = DispatchRaysIndex();
    uint3 lSize = DispatchRaysDimensions();
    
    int samples = 10;
    float3 accuColor = float3(0.0, 0.0, 0.0);
    
    for (int i = 0; i < samples; i++){
        float r1 = random(float2(frac(sin(1.145 * cnt.time) * 7767.42), frac(cos(6.38 * cnt.counter) * 4343.41)));
        float r2 = random(float2(frac(sin(7.321 * cnt.counter) * 3767.42), frac(cos(2.38 * cnt.time) * 1343.41)));
        float2 fwId = float2(lId.xy);
        if (i == 0){
            fwId += float2(0.5, 0.5);
        }
        else{
            fwId += float2(r1, r2);
        }
        float2 uv = fwId / float2(lSize.xy);
        uv = uv * 2.0 - 1.0;
        float4 scrSpacePt = mul(cam.projInv, float4(uv.xy, 1.0, 1.0));
        RayDesc ray;
        ray.Origin = mul(cam.viewInv, float4(0, 0, 0, 1)).xyz;
        ray.Direction = mul(cam.viewInv, float4(normalize(scrSpacePt.xyz), 0)).xyz;
        ray.TMin = 0.001;
        ray.TMax = 10000.0;

        Payload payload;
        payload.iter = 1;
        payload.hitv = float3(0, 0, 0);
        payload.innerIter = i;
        TraceRay(accStruct, 0x01, 0xff, 0, 0, 0, ray, payload);
        accuColor += payload.hitv;
    }
    int od = int(cnt.counter - 0.5);
    
    float share = 1 / (1 + float(od));
   
    float4 orgColor = toneMapping(gammaCorrection(outImage[int2(lId.xy)], true), true);
    float4 newColor = float4(accuColor / float(samples), 1.0) * share + (1 - share) * orgColor;
    newColor = gammaCorrection(newColor, false);
    newColor = toneMapping(newColor, false);
    outImage[int2(lId.xy)] = newColor;

}