struct VSOutput
{
    [[vk::location(0)]] float4 texCoord : TEXCOORD0;
    [[vk::location(1)]] float4 normal : NORMAL0;
    [[vk::location(2)]] float4 rawPos : POSITION0;
    [[vk::location(3)]] float4 ndcPos : POSITION1;
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

struct LightAttrs
{
    int4 volSize;
    float4 lightAttrs; //Density & Anisotropy
    float4 lightColor; //Color & Ambient
    float4 lightDir;
    float4 farNear;
    float4 camPos;
    float4x4 inverseVp;
};


Texture2D texPbrBase[25] : register(t0, space1);
SamplerState sampPbrBase[25] : register(s0, space1);

ConstantBuffer<Camera> camLight : register(b0, space2);
Texture2D texShadow : register(t0, space3);
SamplerState sampShadow : register(s0, space3);

Texture3D texFogVol : register(t0, space4);
SamplerState sampFogVol : register(s0, space4);
ConstantBuffer<LightAttrs> attrs : register(b0, space5);

float4 simpleShadowColor(float4 rawPos, float4 texUV)
{
    int texId = int(texUV.a);
    float4 baseColor = texPbrBase[texId].Sample(sampPbrBase[texId], texUV.xy);
    float4 lightSrcPos = mul(camLight.proj, mul(camLight.view, mul(camLight.model,rawPos)));
    lightSrcPos = lightSrcPos / lightSrcPos.w;
    float2 lightSrcUV = lightSrcPos.xy * 0.5 + 0.5;
    float refLightDepth = texShadow.Sample(sampShadow, lightSrcUV).r;
    float shadow = 0;
    if (lightSrcPos.z > refLightDepth + 1e-4)
    {
        shadow = 1;
    }
    float4 finalColor = (0.2 + 0.8 * (1 - shadow)) * baseColor;
    return finalColor;
}

float3 addVolumeLighting(float4 ndcPos,float4 baseColor)
{
    float3 nNdc = ndcPos.xyz / ndcPos.w;
    nNdc.xy = nNdc.xy * 0.5 + 0.5;
    float ndcZ = (ndcPos.z - attrs.farNear.y) / (attrs.farNear.x - attrs.farNear.y);
    nNdc.z = ndcZ;
    float4 scatterColTrans = texFogVol.Sample(sampFogVol, nNdc);
    float3 scatterColor = scatterColTrans.rgb;
    float3 sourceColor = scatterColTrans.a * baseColor.xyz;
    return scatterColor + sourceColor;
}

float4 revealTransmittance(float4 ndcPos, float4 baseColor)
{
    float3 nNdc = ndcPos.xyz / ndcPos.w;
    nNdc.xy = nNdc.xy * 0.5 + 0.5;
    float ndcZ = (ndcPos.z - attrs.farNear.y) / (attrs.farNear.x - attrs.farNear.y);
    nNdc.z = ndcZ;
    float4 scatterColTrans = texFogVol.Sample(sampFogVol, nNdc);
    return float4(scatterColTrans.a, 0, 0, 0);
}

float4 gammaCorrection(float4 color)
{
    return pow(color, float4(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
}
PSOutput main(VSOutput vsOut)
{
    PSOutput psOut;
    float4 diffuse = simpleShadowColor(vsOut.rawPos, vsOut.texCoord); //TODO: incorrect lighting
    float4 color = float4(addVolumeLighting(vsOut.ndcPos, diffuse), 1.0);
    psOut.color = gammaCorrection(color);
    //psOut.color = revealTransmittance(vsOut.ndcPos, diffuse);
    //psOut.color = diffuse;
    return psOut;
}