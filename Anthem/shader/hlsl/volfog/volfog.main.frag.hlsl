struct VSOutput
{
    [[vk::location(0)]] float4 texCoord : TEXCOORD0;
    [[vk::location(1)]] float4 normal : NORMAL0;
    [[vk::location(2)]] float4 rawPos : POSITION0;
};

struct PSOutput
{
    float4 color : SV_Target0;
};

struct Camera
{
    float4x4 proj;
    float4x4 view;
    float4x4 model;
};

Texture2D texPbrBase[25] : register(t0, space1);
SamplerState sampPbrBase[25] : register(s0, space1);

ConstantBuffer<Camera> camLight : register(b0, space2);
Texture2D texShadow : register(t0, space3);
SamplerState sampShadow : register(s0, space3);

PSOutput main(VSOutput vsOut)
{
    PSOutput psOut;
    int texId = int(vsOut.texCoord.a);
    float4 baseColor = texPbrBase[texId].Sample(sampPbrBase[texId], vsOut.texCoord.xy);
    float4 lightSrcPos = mul(camLight.proj, mul(camLight.view, mul(camLight.model, vsOut.rawPos)));
    lightSrcPos = lightSrcPos / lightSrcPos.w;
    float2 lightSrcUV = lightSrcPos.xy * 0.5 + 0.5;
    float refLightDepth = texShadow.Sample(sampShadow, lightSrcUV).r;
    float shadow = 0;
    if (lightSrcPos.z > refLightDepth+1e-4)
    {
        shadow = 1;
    }
    
    float4 finalColor = (0.2 + 0.8 * (1 - shadow)) * baseColor;
    psOut.color = finalColor;
    return psOut;
}