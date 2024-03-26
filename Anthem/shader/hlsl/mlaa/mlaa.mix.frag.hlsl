struct VSOutput
{
    [[vk::location(0)]] float2 texCoord : TEXCOORD0;
};

Texture2D texBlend : register(t0, space0);
SamplerState sampBlend : register(s0, space0);

Texture2D texSource : register(t0, space1);
SamplerState sampSource : register(s0, space1);


float4 main(VSOutput vsOut):SV_TARGET0{
    
    float texH, texW, texL;
    texSource.GetDimensions(0, texW, texH, texL);
    float dx = 1 / texW, dy = 1 / texH;
    
    float4 v = texBlend.Sample(sampBlend, vsOut.texCoord);
    float4 wR = texBlend.SampleLevel(sampBlend, vsOut.texCoord, 0, int2(1, 0));
    float4 wB = texBlend.SampleLevel(sampBlend, vsOut.texCoord, 0, int2(0, 1));
    
    float4 cC = texSource.Sample(sampSource, vsOut.texCoord);
    float4 cL = texSource.Sample(sampSource, vsOut.texCoord + float2(-dx, 0));
    float4 cR = texSource.Sample(sampSource, vsOut.texCoord + float2(+dx, 0));
    float4 cT = texSource.Sample(sampSource, vsOut.texCoord + float2(0, -dy));
    float4 cB = texSource.Sample(sampSource, vsOut.texCoord + float2(0, dy));
    
    // R: Cur<-Top // G: Cur->Top // B: Cur<-Left // A: Cur->Left
    float4 lL = lerp(cC, cL, v.b);
    float4 lT = lerp(cC, cT, v.r);
    float4 lR = lerp(cC, cR, wR.a);
    float4 lB = lerp(cC, cB, wB.g);
    return (lL + lT + lR + lB) / 4;
}