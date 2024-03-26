struct VSOutput
{
    [[vk::location(0)]] float2 texCoord : TEXCOORD0;
};

Texture2D texTarget : register(t0, space0);
SamplerState sampTarget : register(s0, space0);

Texture2D texHistory : register(t0, space1);
SamplerState sampHistory : register(s0, space1);

Texture2D texMotion : register(t0, space2);
SamplerState sampMotion : register(s0, space2);

Texture2D texDepth : register(t0, space3);
SamplerState sampDepth : register(s0, space3);

static const int FILTER_RANGE = 1;
static const float MOTION_MODIFIER = 100;

float2 getMotionInfo(float2 texc){
    float texH, texW, texL;
    texMotion.GetDimensions(0, texW, texH, texL);
    float dx = 1 / texW, dy = 1 / texH;
    
    float2 depthSampleLoc = texc;
    float depthCur = texDepth.Sample(sampDepth, texc).r;
    for (int sx = -FILTER_RANGE; sx <= FILTER_RANGE; sx++){
        for (int sy = -FILTER_RANGE; sy <= FILTER_RANGE; sy++){
            float cDepth = texDepth.Sample(sampDepth, float2(texc.x + float(sx) * dx, texc.y + float(sy) * dy)).r;
            if (cDepth < depthCur){
                depthSampleLoc = texc;
                depthCur = cDepth;
            }
        }
    }
    return depthSampleLoc;
}

float4 rgb2YCgCo(float4 color){
    float y = dot(float3(0.25, 0.5, 0.25), color.xyz);
    float cg = dot(float3(-0.25, 0.5, -0.25), color.xyz);
    float co = dot(float3(0.5, 0.0, -0.5), color.xyz);
    return float4(y, cg, co,1);
}

float4 yCgCo2Rgb(float4 color){
    float y = color.r, cg = color.g, co = color.b;
    float g = y + cg;
    float r = y + co - cg;
    float b = y - co - cg;
    return float4(r, g, b, 1);
}

float4 clampColorInRegion(float2 texc, float4 color){
    float texH, texW, texL;
    texMotion.GetDimensions(0, texW, texH, texL);
    float dx = 1 / texW, dy = 1 / texH;
    
    float4 maxColor = float4(0, 0, 0, 0);
    float4 minColor = float4(1, 1, 1, 1);
    for (int sx = -FILTER_RANGE; sx <= FILTER_RANGE; sx++){
        for (int sy = -FILTER_RANGE; sy <= FILTER_RANGE; sy++){
            float2 curc = float2(float(sx) * dx, float(sy) * dy) + texc;
            float4 curColor = rgb2YCgCo(texTarget.Sample(sampTarget, curc));
            maxColor = max(curColor, maxColor);
            minColor = min(curColor, minColor);
        }
    }
    return clamp(color, minColor, maxColor);
}

float4 main(VSOutput vsOut) : SV_Target0{
    float2 motionVecLoc = getMotionInfo(vsOut.texCoord);
    float4 motionData = texMotion.Sample(sampMotion, motionVecLoc);
    motionData = float4(motionData.xyz, 1);
    float2 motionVec = 2 * (motionData - 0.5).xy;
    if (motionData.z < 0.5){
        motionVec = float2(0, 0);
    }
    
    float4 historyColor = texHistory.Sample(sampHistory, vsOut.texCoord - motionVec*0.5);
    historyColor = rgb2YCgCo(historyColor);
    historyColor = clampColorInRegion(vsOut.texCoord, historyColor);
    historyColor = yCgCo2Rgb(historyColor);
    
    float curContribFactor = saturate(0.05 + 100 * length(motionVec));
    float4 curColor = texTarget.Sample(sampTarget, vsOut.texCoord);
    return lerp(historyColor, curColor, curContribFactor);
}