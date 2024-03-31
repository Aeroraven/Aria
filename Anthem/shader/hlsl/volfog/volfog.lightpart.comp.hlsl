struct LightAttrs
{
    int4 volSize;
    float4 lightAttrs;
    float4 lightColor;
    float4x4 inverseVp;
};

ConstantBuffer<LightAttrs> attr : register(b0, space0);
RWTexture3D<float4> scatData : register(u0, space1);

[numthreads(8, 8, 8)]
void main(uint3 invId : SV_DispatchThreadID)
{
    scatData[invId] = float4(1, 0, 0, 0);
}