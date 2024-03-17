struct VSOutput
{
    [[vk::location(0)]] float4 orgPosition : POSITION0;
    [[vk::location(1)]] float4 texCoord : TEXCOORD0;
    [[vk::location(2)]] float4 normal : NORMAL0;
};

struct Camera
{
    float4x4 proj;
    float4x4 view;
    float4x4 model;
};

struct Light
{
    float4 pos;
    float4 ambient;
};

struct PSOutput
{
    float4 color : SV_Target0;
};

TextureCube texShadowMap : register(t0, space1);
SamplerState sampShadowMap : register(s0, space1);
ConstantBuffer<Camera> lightCam[6] : register(b0, space2);
ConstantBuffer<Light> lightConf : register(b0, space3);
Texture2D texImage[4] : register(t0, space4);
SamplerState sampImage[4] : register(s0, space4);

float calculateShadow(float4 pos)
{
    float3 dirx = normalize(pos.xyz - lightConf.pos.xyz);
    float refDepth = texShadowMap.Sample(sampShadowMap, dirx).r;
    float curDepth = length(pos.xyz - lightConf.pos.xyz) / 1000.0;

    if (curDepth - refDepth > 1e-6)
    {
        return 1.0;
    }
    return 0.0;
}

PSOutput main(VSOutput vsOut)
{
    PSOutput psOut;
    float3 inRay = normalize(lightConf.pos.xyz - vsOut.orgPosition.xyz);
    float3 rfNorm = normalize(vsOut.normal.xyz);
    float diffuse = max(0.0, dot(inRay, rfNorm));
    float4 baseColor = float4(1.0, 1.0, 1.0, 1.0);
    if (vsOut.texCoord.z < 4.0)
    {
        int index = int(vsOut.texCoord.z);
        baseColor = texImage[index].Sample(sampImage[index], vsOut.texCoord.xy);
    }
    float shadow = calculateShadow(vsOut.orgPosition);
    
    psOut.color = ((1.0 - shadow) * diffuse + lightConf.ambient) * baseColor;
    psOut.color.a = 1.0;
    return psOut;
}