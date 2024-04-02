struct VSOutput
{
    [[vk::location(0)]] float4 texCoord : TEXCOORD0;
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
    float4 jitter;
    float4x4 inverseVp;
};

static const int PCF_RANGE = 2;


ConstantBuffer<Camera> camLight : register(b0, space1);
Texture2D texShadow : register(t0, space2);
SamplerState sampShadow : register(s0, space2);

Texture3D texFogVol : register(t0, space3);
SamplerState sampFogVol : register(s0, space3);
ConstantBuffer<LightAttrs> attrs : register(b0, space4);

Texture2D texBaseColor: register(t0, space5);
SamplerState sampBaseColor : register(s0, space5);
Texture2D texNorm : register(t0, space6);
SamplerState sampNorm : register(s0, space6);
Texture2D texPos : register(t0, space7);
SamplerState sampPos : register(s0, space7);
Texture2D texNdc : register(t0, space8);
SamplerState sampNdc : register(s0, space8);
Texture2D texAO : register(t0, space9);
SamplerState sampAO : register(s0, space9);


float pcfShadowMap(float2 uv,float curDepth)
{
    float texH, texW, texL;
    texShadow.GetDimensions(0,texH, texW, texL);
    float dx = 1 / texW, dy = 1 / texH;
    
    float totalShadow = 0;
    float adoptedShadow = 0;
    for (int i = -PCF_RANGE; i <= PCF_RANGE; i++)
    {
        for (int j = -PCF_RANGE; j <= PCF_RANGE; j++)
        {
            totalShadow += 1.0;
            float refLightDepth = texShadow.Sample(sampShadow, uv + float2(float(i) * dx, float(j) * dy)).r;
            if (curDepth > refLightDepth + 1e-4)
            {
                adoptedShadow += 1.0;
            }
        }
    }
    return adoptedShadow / totalShadow;
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

float4 toneMapping(float4 color)
{
    return color / (color + 1.0);
}

float4 gammaCorrection(float4 color)
{
    return pow(color, float4(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
}
PSOutput main(VSOutput vsOut)
{
    PSOutput psOut;

    float4 adjNormal = texNorm.Sample(sampNorm, vsOut.texCoord.xy);
    float4 basecolor = texBaseColor.Sample(sampBaseColor, vsOut.texCoord.xy);
    float4 ndcPos = texNdc.Sample(sampNdc, vsOut.texCoord.xy);
    float4 rawPos = texPos.Sample(sampPos, vsOut.texCoord.xy);
    float4 ao = texAO.Sample(sampAO, vsOut.texCoord.xy);
    
    float4 lightSrcPos = mul(camLight.proj, mul(camLight.view, mul(camLight.model, rawPos)));
    lightSrcPos = lightSrcPos / lightSrcPos.w;
    float2 lightSrcUV = lightSrcPos.xy * 0.5 + 0.5;
    float shadow = pcfShadowMap(lightSrcUV, lightSrcPos.z);
    
    float diffuse = max(0, dot(normalize(adjNormal.xyz), -normalize(attrs.lightDir.xyz)));
    float4 baselightColor;
    float ambient = attrs.lightColor.a * (1 - ao.r);
    baselightColor = (ambient + (1 - shadow) * diffuse) * basecolor;
    
    float4 color = float4(addVolumeLighting(ndcPos, baselightColor), 1.0);
    psOut.color = gammaCorrection(toneMapping(color));

    return psOut;
}