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

static const float3 LIGHT_DIR = normalize(float3(1, -1, -1));

float4 gamma(float4 color)
{
    return pow(color, 1.0 / 2.2);
}

PSOutput main(VSOutput vsOut)
{
    PSOutput psOut;
    float4 posr = texPosition.Sample(sampPosition, vsOut.texUv.xy).xyzw;
    if(posr.a < 0.5)
    {
        psOut.color = float4(0, 0, 0, 0);
        return psOut;
    }
    if (posr.a > 1.5)
    {
        psOut.color = texColor.Sample(sampColor, vsOut.texUv.xy);
        return psOut;
    }
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
    psOut.color = gamma(psOut.color);
    
    return psOut;
}