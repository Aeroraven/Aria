struct VSOutput
{
    [[vk::location(0)]] float2 texCoord : TEXCOORD0;
};


struct PushConst
{
    float cIter;
};

[[vk::push_constant]] PushConst pc;

Texture2D texIn : register(t0, space0);
SamplerState sampIn : register(s0, space0);

float4 downSampleBlur(float2 texc)
{
    float texH, texW, texLods;
    texIn.GetDimensions(0, texW, texH, texLods);
    float dx = 1.0 / texW;
    float dy = 1.0 / texH;
    float mult = 1.0;
    
    float4 llt = texIn.Sample(sampIn, texc + float2(-0.5 - pc.cIter * mult, -0.5 - pc.cIter * mult) * float2(dx, dy));
    float4 lrt = texIn.Sample(sampIn, texc + float2(0.5 + pc.cIter * mult, -0.5 - pc.cIter * mult) * float2(dx, dy));
    float4 llb = texIn.Sample(sampIn, texc + float2(-0.5 - pc.cIter * mult, 0.5 + pc.cIter * mult) * float2(dx, dy));
    float4 lrb = texIn.Sample(sampIn, texc + float2(0.5 + pc.cIter * mult, 0.5 + pc.cIter * mult) * float2(dx, dy));
    return (llt + lrt + llb + lrb) * 0.25;
    
}

float4 main(VSOutput vsOut) : SV_Target0
{
    float threshold = texIn.Sample(sampIn, vsOut.texCoord).a;
    threshold = pc.cIter <= 0.5 ? smoothstep(0.1, 0.2, threshold) : 1.0;
    return downSampleBlur(vsOut.texCoord) * threshold;
}