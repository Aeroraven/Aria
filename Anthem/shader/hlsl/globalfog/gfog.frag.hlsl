struct VSOutput
{
    [[vk::location(0)]] float2 texCoord : TEXCOORD0;
};

Texture2D texCol : register(t0, space0);
SamplerState sampCol : register(s0, space0);
Texture2D texPos : register(t0, space1);
SamplerState sampPos : register(s0, space1);


static const float SCATTER_F = 0.015;
static const float HEIGHT_ATTN = 0.3;
static const float ELEVATION = 0;
static const float4 FOG_COLOR = float4(0.95, 0.95, 0.95, 1);

struct Attr
{
    float4 val; //SCATTER, ATTN
};
ConstantBuffer<Attr> attr : register(b0, space2);

float4 toLinear(float4 x)
{
    return pow(x, float4(2.2, 2.2, 2.2, 2.2));
}
float4 toSrgb(float4 x)
{
    return pow(x, float4(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
}

float4 main(VSOutput vsOut) : SV_Target0
{
    float3 pos = texPos.Sample(sampPos, vsOut.texCoord).rgb;
    float dist = distance(pos, float3(0, ELEVATION, 0));
    float distH = max(1e-3,pos.y - ELEVATION);
    float sinv = distH / dist;
    
    float4 baseColor = toLinear(texCol.Sample(sampCol, vsOut.texCoord));
    float thickness = attr.val.x * exp(-attr.val.y * ELEVATION) * (1.0 - exp(-attr.val.y * dist * sinv)) / (attr.val.y * sinv);

    float coef = 1 - exp(-thickness);
    
    float4 finalColor = lerp(baseColor, FOG_COLOR, float4(coef, coef, coef, coef));
    return toSrgb(finalColor);

}