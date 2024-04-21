
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
float4 biomeRamp(float height)
{
    float transitionMod = 0.12;
    
    float layerTop = 300;
    float layerRidge = 260;
    float layerForest = 120;
    float layerPlain = 70;
    float layerBeach = 0;
    
    float weightTop = sigmoidMaginfy(height - layerTop, transitionMod);
    float weightRidge = sigmoidMaginfy(height - layerRidge, transitionMod) - weightTop;
    float weightForest = sigmoidMaginfy(height - layerForest, transitionMod) - weightTop - weightRidge;
    float weightPlain = sigmoidMaginfy(height - layerPlain, transitionMod) - weightTop - weightRidge - weightForest;
    float weightBeach = sigmoidMaginfy(height - layerBeach, transitionMod) - weightTop - weightRidge - weightForest - weightPlain;
    
    float4 baseColorTop = float4(0.95, 0.95, 0.95, 1);
    float4 baseColorRidge = float4(0.5, 0.5, 0.5, 1);
    float4 baseColorForest = float4(0.1, 0.5, 0.1, 1);
    float4 baseColorPlain = float4(0.4, 0.8, 0.4, 1);
    float4 baseColorBeach = float4(0.9, 0.9, 0.5, 1);
    
    float4 mixedColor = baseColorTop * weightTop;
    mixedColor += baseColorRidge * weightRidge;
    mixedColor += baseColorForest * weightForest;
    mixedColor += baseColorPlain * weightPlain;
    mixedColor += baseColorBeach * weightBeach;
    
    return mixedColor;
}

PSOutput main(VSOutput vsOut)
{
    PSOutput psOut;
    psOut.position = vsOut.position;
    psOut.normal = vsOut.normal;
    psOut.color = biomeRamp(vsOut.position.y);
    psOut.tangent = vsOut.tangent;
    return psOut;
}