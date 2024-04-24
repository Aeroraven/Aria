#include "pt.common.hlsl"

struct VSOutput
{
    [[vk::location(0)]] float4 texUv : TEXCOORD0;
};
struct PSOutput
{
    float4 color : SV_Target0;
};

Texture2D texColor : register(t0, space0);
SamplerState sampColor : register(s0, space0);
Texture2D texNormal : register(t0, space1);
SamplerState sampNormal : register(s0, space1);
Texture2D texPosition : register(t0, space2);
SamplerState sampPosition : register(s0, space2);
Texture2D texAO : register(t0, space3);
SamplerState sampAO : register(s0, space3);
ConstantBuffer<Camera> cam:register(b0, space4);

Texture2D texWaterMask : register(t0, space5);
SamplerState sampWaterMask : register(s0, space5);
TextureCube texSkybox : register(t0, space6);
SamplerState sampSkybox : register(s0, space6);

Texture2D texWaterNormal : register(t0, space7);
SamplerState sampWaterNormal : register(s0, space7);
Texture2D texWaterDuDv : register(t0, space8);
SamplerState sampWaterDuDv : register(s0, space8);
Texture2D texWaterUV : register(t0, space9);
SamplerState sampWaterUV : register(s0, space9);

static const float3 LIGHT_DIR = normalize(float3(0, -1, -1));
static const float3 WATER_NORMAL = normalize(float3(0, 1, 0));
static const float SSR_MAXDIST = 1500.0;
static const float SSR_DEPTH_THRESHOLD = 0.5;
static const float3 WATER_COLOR = float3(0.13, 0.54, 0.89);
static const float WATER_REFRAC_INDICE = 0.1427;
static const float WATER_SIZE = 1600;
static const float WATER_ELEVATION = 50;
static const float WATER_WAVE_STRENGTH = 0.003;
static const float WATER_NORMAL_OFFSET = 0.03;
static const float SPECULAR_HIGHLIGHT = 50.0;
static const float4 WHITE = float4(1, 1, 1, 1);
float4 gamma(float4 color)
{
    return pow(color, 1.0 / 2.2);
}

float schlickFresnel(float f0, float cA)
{
    return f0 + (1 - f0) * pow(1 - cA, 5.0);
}

float aoBlur(float2 uv)
{
    float texH, texW, texLod;
    texAO.GetDimensions(0, texW, texH, texLod);
    float2 texelSize = 1.0 / float2(texW, texH);
    
    float color =0;
    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            color += texAO.Sample(sampAO, uv + float2(x, y) * texelSize).r;
        }
    }
    return color / 9.0;
}


float4 pointColor(float2 uv,float4 posr)
{
    float4 ret;
    
    //Background -> Discard
    if (posr.a < 0.5)
    {
        ret = float4(0, 0, 0, 0);
        return ret;
    }
    //Skybox
    if (posr.a > 1.5)
    {
        ret = texColor.Sample(sampColor,uv);
    }

    // Objects
    else
    {
        float3 pos = posr.xyz;
        float3 normal = texNormal.Sample(sampNormal, uv).xyz;
        float4 color = texColor.Sample(sampColor, uv);
        float ao = aoBlur(uv);
    
        // Diffuse
        float3 lightDir = -LIGHT_DIR;
        float diff = max(0, dot(normal, lightDir));
    
        // Ambient
        float ambient = 0.4;
    
        float3 finalColor = color.rgb * (ambient + diff) * ao;
        ret = float4(finalColor, 1.0);
    }
    return ret;
}

float4 getWaterPosition(float2 uv)
{
    float posX = lerp(-WATER_SIZE, WATER_SIZE, uv.x);
    float posY = lerp(-WATER_SIZE, WATER_SIZE, uv.y);
    return float4(posX, WATER_ELEVATION, posY, 1);
}
float2 getUVFromPosition(float4 pos)
{
    float uvX = pos.x / (2.0 * WATER_SIZE) + 0.5;
    float uvY = pos.z / (2.0 * WATER_SIZE) * 0.5;
    return float2(uvX, uvY);
}

PSOutput main(VSOutput vsOut)
{
    PSOutput psOut;
    
    float4 posr = texPosition.Sample(sampPosition, vsOut.texUv.xy).xyzw;
    psOut.color = pointColor(vsOut.texUv.xy, posr);
    
    // Water Blending
    float4 waterData = texWaterMask.Sample(sampWaterMask, vsOut.texUv.xy).rgba;
    float2 waterUV = texWaterUV.Sample(sampWaterUV, vsOut.texUv.xy).xy;
    
    float timeStep = frac(cam.timer.x * 0.015);
    float2 waterDistortion1 = texWaterDuDv.Sample(sampWaterDuDv, frac((vsOut.texUv.xy + float2(timeStep, 0)) * 3.0)).xy * 2.0 - 1.0;
    float2 waterDistortion2 = texWaterDuDv.Sample(sampWaterDuDv, frac((vsOut.texUv.xy * float2(-1, 0) + float2(0, timeStep)) * 3.0)).xy * 2.0 - 1.0;
    float2 waterDistortion = waterDistortion1 + waterDistortion2;
    
    float2 waterNewUV = waterUV + waterDistortion * WATER_WAVE_STRENGTH;
    float4 waterPos = getWaterPosition(waterNewUV);
    
    float waterMask = waterData.a;
    if (waterMask < 0.1)
    {
        psOut.color = gamma(psOut.color);
        return psOut;
    }

    // Get Distorted NDC
    float4 waterPosNdc = mul(cam.proj, mul(cam.view, mul(cam.model, float4(waterPos.xyz, 1))));
    float2 reprojUV = ((waterPosNdc) / waterPosNdc.w).xy * 0.5 + 0.5;
    psOut.color = pointColor(reprojUV, texPosition.Sample(sampPosition, reprojUV));
    
    float3 intersectionPos = waterPos.xyz;
    float3 waterNormal = texWaterNormal.Sample(sampWaterNormal, frac(waterNewUV * 3.0)).rgb;
    waterNormal.xz = (waterNormal.xz * 2 - 1) * WATER_NORMAL_OFFSET;
    waterNormal = normalize(waterNormal);
    
    float3 viewDir = normalize(cam.camPos.xyz - intersectionPos);
    float3 reflectDir = normalize(reflect(-viewDir, waterNormal));
    
    float4 skyboxColor = texSkybox.Sample(sampSkybox, reflectDir);
    
    // Screen Space Reflection
    float3 startPosWS = intersectionPos;
    float4 startPosSSH = mul(cam.proj, mul(cam.view, float4(startPosWS, 1.0)));
    float3 startPosSS = startPosSSH.xyz / startPosSSH.w;
    
    float3 endPosWS = startPosWS + reflectDir * SSR_MAXDIST;
    float4 endPosSSH = mul(cam.proj, mul(cam.view, float4(endPosWS, 1.0)));
    float3 endPosSS = endPosSSH.xyz / endPosSSH.w;
    
    float texW, texH, texLods;
    texColor.GetDimensions(0, texW, texH, texLods);
    
    float2 startPx = startPosSS.xy * 0.5 + 0.5;
    float2 endPx = endPosSS.xy * 0.5 + 0.5;
    
    float2 deltaCoord = abs(endPx - startPx) * float2(texW, texH);
    int deltaSel = int(max(deltaCoord.x, deltaCoord.y)); 
    int hit = -1;
    float4 reflColor = skyboxColor;
    float diff = 10000.0;
    int i = 0;
    
    float invCamDepth = cam.camPos.z;
    for (i = 1; i < deltaSel; i+=2)
    {
        float percent = float(i) / float(deltaSel);
        float2 curPx = lerp(startPx,endPx, percent);
        if(curPx.x < 0 || curPx.x > 1 || curPx.y < 0 || curPx.y > 1)
            break;
        float curDepthRef = (startPosWS.z - invCamDepth) * (endPosWS.z - invCamDepth) / lerp(endPosWS.z - invCamDepth, startPosWS.z - invCamDepth, percent);
        float4 curToPos = texPosition.Sample(sampPosition, curPx);
        float curDepth = curToPos.z - invCamDepth;
        if (curToPos.a > 1.5 || curToPos.y < WATER_ELEVATION)
        {
            continue;
        }
        
        float deltaDepth = curDepth - curDepthRef;
        if (deltaDepth < 0 && abs(deltaDepth)<10.0)
        {
            hit = i;
            float4 dPosr = texPosition.Sample(sampPosition, curPx);
            reflColor = pointColor(curPx, dPosr); //texColor.Sample(sampColor, curPx);
            break;
        }

    }
    
    // Blending Refraction and Reflection
    float3 refractColor = psOut.color.rgb;
    float3 reflectColor = reflColor.rgb;
    float3 waterColor = WATER_COLOR;
    
    float ndotl = max(0, dot(waterNormal, viewDir));
    float3 h = normalize(-LIGHT_DIR + viewDir);
    float ndoth = max(0, dot(waterNormal, h));
    float specular = pow(ndoth, SPECULAR_HIGHLIGHT);
    
    
    float fresnel = schlickFresnel(WATER_REFRAC_INDICE, ndotl);
    float3 transColor = lerp(refractColor, reflectColor, fresnel);
    
    psOut.color.rgb = lerp(transColor, waterColor, 0.3) + specular;
    
    // Final Color
    psOut.color = gamma(psOut.color);
    
    return psOut;
}