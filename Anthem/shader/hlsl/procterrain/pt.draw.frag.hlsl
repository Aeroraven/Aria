#include "pt.noise.hlsl"

struct VSOutput
{
    [[vk::location(0)]] float4 normal : NORMAL0;
    [[vk::location(1)]] float4 position : POSITION0;
    [[vk::location(2)]] float4 tangent : TANGENT0;
};

struct PSOutput
{
    float4 color : SV_Target0;
    float4 normal : SV_Target1;
    float4 position : SV_Target2;
    float4 tangent : SV_Target3;
};
float sigmoidMaginfy(float v, float mag)
{
    return 1.0 / (1.0 + exp(-v * mag));
}
float4 fromRGB(float r,float g,float b)
{
    return float4(r / 255.0, g / 255.0, b / 255.0, 1);

}
float snowPlacement(float2 pos)
{
    float sumAmpl = 0;
    float ampl = 1;
    float freq = 1;
    float value = 0;
    for (int i = 0; i < 8; i++)
    {
        value += cnoise(float3(pos * 0.04 * freq, 1)) * ampl;
        sumAmpl += ampl;
        ampl *= 0.3;
        freq *= 2;

    }
    return value / sumAmpl;
}
float4 biomeRamp(float height,float3 normal,float3 pos)
{
    // Noise
    float noise = snowPlacement(pos.xz);
    
    // Base Color
    float transitionMod = 0.62;
    float transitionModSnow = 0.92;
    
    float layerTop = 490 - noise * 80;
    float layerRidge = 260;
    float layerPlain = 70;
    float layerBeach = 0;
    
    float weightTop = sigmoidMaginfy(height - layerTop, transitionModSnow);
    float weightRidge = sigmoidMaginfy(height - layerRidge, transitionMod) - weightTop;
    float weightPlain = sigmoidMaginfy(height - layerPlain, transitionMod) - weightTop - weightRidge;
    float weightBeach = sigmoidMaginfy(height - layerBeach, transitionMod) - weightTop - weightRidge - weightPlain;
    
    float4 baseColorTop = float4(0.98, 0.98, 0.98, 1);
    float4 baseColorRidge = fromRGB(97, 93, 87);
    float4 baseColorPlain = fromRGB(123, 181, 90)*0.9;
    float4 baseColorBeach = fromRGB(239, 239, 169);
    
    // Normal Facing Checks
    if (height < layerTop)
    {
        float updir = dot(normalize(normal), float3(0, 1, 0));
        float plainCoef = smoothstep(0.73, 0.78, updir);
        float prSum = weightPlain + weightRidge + weightBeach;
        float prBeachShare = weightBeach / (weightPlain + weightBeach+1e-3);
        weightPlain = (plainCoef) * prSum * (1 - prBeachShare);
        weightBeach = (plainCoef) * prSum * prBeachShare;
        weightRidge = (1 - plainCoef) * prSum;
        
        // Reweighting for Beach
        prSum =  weightBeach;
        float beachCoef = smoothstep(0.80, 0.82, updir);
        weightBeach = beachCoef * prSum;
        weightRidge += (1 - beachCoef) * prSum;

    }
    else if (height>layerTop)
    {
        float updir = dot(normalize(normal), float3(0, 1, 0));
        float snowCoef = clamp((smoothstep(0.18, 0.19, updir + (height-layerTop) * 0.0005)), 0, 1);
        float prSum = weightTop;
        weightTop = (snowCoef) * prSum;
        weightRidge += (1 - snowCoef) * prSum;
    }

    // For Snow
    
    
    float4 mixedColor = baseColorTop * weightTop;
    mixedColor += baseColorRidge * weightRidge;
    mixedColor += baseColorPlain * weightPlain;
    mixedColor += baseColorBeach * weightBeach;
    
    return mixedColor;
}

PSOutput main(VSOutput vsOut)
{
    PSOutput psOut;
    psOut.position = vsOut.position;
    psOut.normal = vsOut.normal;
    psOut.color = biomeRamp(vsOut.position.y,vsOut.normal.xyz,vsOut.position.xyz);
    psOut.tangent = vsOut.tangent;
    return psOut;
}