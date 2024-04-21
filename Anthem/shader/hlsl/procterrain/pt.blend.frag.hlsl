struct VSOutput
{
    [[vk::location(0)]] float4 texUv : TEXCOORD0;
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
    float4 camPos;
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


static const float3 LIGHT_DIR = normalize(float3(1, -1, -1));
static const float3 WATER_NORMAL = normalize(float3(0, 1, 0));
static const float SSR_MAXDIST = 500.0;
static const float SSR_DEPTH_THRESHOLD = 0.5;
static const float3 WATER_COLOR = float3(0.13, 0.54, 0.89);

float4 gamma(float4 color)
{
    return pow(color, 1.0 / 2.2);
}

PSOutput main(VSOutput vsOut)
{
    PSOutput psOut;
    float4 posr = texPosition.Sample(sampPosition, vsOut.texUv.xy).xyzw;
    
    //Background -> Discard
    if(posr.a < 0.5)
    {
        psOut.color = float4(0, 0, 0, 0);
        return psOut;
    }
    //Skybox
    if (posr.a > 1.5)
    {
        psOut.color = float4(0, 0, 0, 0); //texColor.Sample(sampColor, vsOut.texUv.xy);

    }

    // Objects
    else
    {
        float3 pos = posr.xyz;
        float3 normal = texNormal.Sample(sampNormal, vsOut.texUv.xy).xyz;
        float4 color = texColor.Sample(sampColor, vsOut.texUv.xy);
        float ao = texAO.Sample(sampAO, vsOut.texUv.xy).r;
    
        // Diffuse
        float3 lightDir = -LIGHT_DIR;
        float diff = max(0, dot(normal, lightDir));
    
        // Ambient
        float ambient = 0.1;
    
        float3 finalColor = color.rgb * (ambient + diff) * ao;
        psOut.color = float4(finalColor, 1.0);
    }
    
    // Water Blending
    float4 waterData = texWaterMask.Sample(sampWaterMask, vsOut.texUv.xy).rgba;
    float waterMask = waterData.a;
    if (waterMask < 0.1)
    {
        psOut.color = float4(0, 0, 0, 0);
        psOut.color = gamma(psOut.color);
        return psOut;
    }

    
    float3 intersectionPos = waterData.rgb;
    float3 waterNormal = WATER_NORMAL;
    float3 viewDir = normalize(cam.camPos.xyz - intersectionPos);
    float3 reflectDir = reflect(-viewDir, waterNormal);
    
    // Screen Space Reflection
    float3 startPosWS = intersectionPos;
    float4 startPosSSH = mul(cam.proj, mul(cam.view, float4(startPosWS, 1.0)));
    float3 startPosSS = startPosSSH.xyz / startPosSSH.w;
    
    float3 endPosWS = startPosWS + reflectDir * SSR_MAXDIST;
    float4 endPosSSH = mul(cam.proj, mul(cam.view, float4(endPosWS, 1.0)));
    float3 endPosSS = endPosSSH.xyz / endPosSSH.w;
    
    float3 dirSS = normalize(endPosSS - startPosSS);
    float3 curPos = startPosSS + dirSS * (1e-2);
    
    float texW, texH, texLods;
    texColor.GetDimensions(0, texW, texH, texLods);
    float tDx = 1.0 / texW;
    float tDy = 1.0 / texH;
    
    float2 startPx = startPosSS.xy * 0.5 + 0.5;
    float2 endPx = endPosSS.xy * 0.5 + 0.5;
    
    float2 deltaCoord = abs(endPx-startPx) / float2(tDx, tDy);
    int deltaSel = int(max(deltaCoord.x, deltaCoord.y));
    int hit = 0;
    float4 reflColor = float4(1, 0, 0, 0);
    float diff = 10000.0;
    int i = 0;
    for (i = 0; i < deltaSel; i++)
    {
        //diff = min(diff, 9000);
        float percent = float(i) / float(deltaSel);
        float2 curPx = lerp(startPx,endPx, percent);
        if(curPx.x < 0 || curPx.x > 1 || curPx.y < 0 || curPx.y > 1)
            break;
        float curDepthRef = startPosWS.z*endPosWS.z/lerp(endPosWS.z, startPosWS.z, percent);
        float curDepth = texPosition.Sample(sampPosition, curPx).z;
        
        float deltaDepth = curDepth - curDepthRef;
        diff = min(abs(diff), deltaDepth);
        if (abs(deltaDepth) < SSR_DEPTH_THRESHOLD)
        {
            hit = 1;
            reflColor = texColor.Sample(sampColor, curPx);
        }

    }
    
    // Blending Refraction and Reflection
    float3 refractColor = psOut.color.rgb;
    float3 reflectColor = reflColor.rgb;
    float3 waterColor = WATER_COLOR;
    
    psOut.color.rgb = diff; //lerp(reflectColor, float3(0, 0, 1), waterMask * 0.8);
    
    // Final Color
    //psOut.color = gamma(psOut.color);
    
    return psOut;
}