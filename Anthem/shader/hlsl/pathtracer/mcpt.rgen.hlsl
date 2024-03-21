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
};

ConstantBuffer<Counter> cnt : register(b0, space9);
ConstantBuffer<Camera> cam : register(b0, space2);
struct Payload
{
    [[vk::location(0)]] float3 hitv;
    [[vk::location(1)]] uint iter;
    [[vk::location(2)]] uint innerIter;
};

float4 gammaCorrection(float4 base)
{
    float b = 1.0 / 2.2;
    return pow(base, float4(b, b, b, b));
}

[shader("raygeneration")]
void main()
{
    uint3 lId = DispatchRaysIndex();
    uint3 lSize = DispatchRaysDimensions();
    
    float2 fwId = float2(lId.xy) + float2(0.5, 0.5);
    float2 uv = fwId / float2(lSize.xy);
    uv = uv * 2.0 - 1.0;
    int samples = 5;
    float3 accuColor = float3(0.0, 0.0, 0.0);
    for (int i = 0; i < samples; i++){
        float4 scrSpacePt = mul(cam.projInv, float4(uv.xy, 1.0, 1.0));
        RayDesc ray;
        ray.Origin = mul(cam.viewInv, float4(0, 0, 0, 1)).xyz;
        ray.Direction = mul(cam.viewInv, float4(normalize(scrSpacePt.xyz), 0)).xyz;
        ray.TMin = 0.001;
        ray.TMax = 10000.0;

        Payload payload;
        payload.iter = 1;
        payload.innerIter = i;
        TraceRay(accStruct, 0x01, 0xff, 0, 0, 0, ray, payload);
        accuColor += payload.hitv;
    }
    int od = int(cnt.counter - 0.5);
    
    float share = 1 / (float(od) + 1);

    outImage[int2(lId.xy)] = gammaCorrection(float4(accuColor / float(samples), 0.0)) * share + (1 - share) * outImage[int2(lId.xy)];
}