struct LightAttrs
{
    int4 volSize;
    float4 lightAttrs;
    float4 lightColor;
    float4x4 inverseVp;
};

ConstantBuffer<LightAttrs> attr : register(b0, space0);
RWTexture3D<float4> fogData : register(u0, space1);

[numthreads(8, 8, 1)]
void main(uint3 invId : SV_DispatchThreadID)
{
    fogData[invId] = float4(1, 0, 0, 0);
}