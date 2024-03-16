struct VSOutput
{
    [[vk::location(0)]] float4 texCoord : TEXCOORD0;
};

struct PSOutput
{
    float4 color : SV_Target0;
};


TextureCube texSkybox : register(t0, space1);
SamplerState sampSkybox : register(s0, space1);

struct Camera
{
    float4x4 proj;
    float4x4 view;
    float4x4 model;
};

ConstantBuffer<Camera> lcm[6] : register(b0, space2);


PSOutput main(VSOutput vsOut)
{
    PSOutput psOut;
    psOut.color = float4(texSkybox.Sample(sampSkybox, vsOut.texCoord.xyz).rgb, 1.0) * 1.0;// + 0.5 * float4(vsOut.texCoord.xyz, 1.0);
    //psOut.color = abs(float4(lcm[4].view[3][3],0.0,0.0,1.0));
    return psOut;
}