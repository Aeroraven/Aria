RaytracingAccelerationStructure accStruct : register(t0,space0);
RWTexture2D<float4> outImage : register(u0,space1);
struct Camera
{
    float4x4 projInv;
    float4x4 viewInv;
};

ConstantBuffer<Camera> cam : register(b0, space2);
struct Payload
{
    [[vk::location(0)]] float3 hitv;
};

[shader("raygeneration")]
void main()
{
    uint3 lId = DispatchRaysIndex();
    uint3 lSize = DispatchRaysDimensions();
    
    float2 fwId = float2(lId.xy) + float2(0.5, 0.5);
    float2 uv = fwId / float2(lSize.xy);
    uv = uv * 2.0 - 1.0;
    float4 scrSpacePt = mul(cam.projInv, float4(uv.xy, 1.0, 1.0));
    
    RayDesc ray;
    ray.Origin = mul(cam.viewInv , float4(0, 0, 0, 1)).xyz;
    ray.Direction = mul(cam.viewInv, float4(normalize(scrSpacePt.xyz), 0)).xyz;
    ray.TMin = 0.001;
    ray.TMax = 10000.0;

    Payload payload;
    TraceRay(accStruct, 0x01, 0xff, 0, 0, 0, ray, payload);

    outImage[int2(lId.xy)] = float4(payload.hitv, 0.0);
}