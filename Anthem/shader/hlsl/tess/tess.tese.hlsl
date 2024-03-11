struct HSOutput
{
    float4 position : SV_Position;
    [[vk::location(0)]] float2 texCoord : TEXCOORD0;
};

struct ConstHSOutput
{
    float tessLvlOuter[4] : SV_TessFactor;
    float tessLvlInner[2] : SV_InsideTessFactor;
};

struct UniformBuffer
{
    [[vk::location(0)]] float4x4 proj;
    [[vk::location(1)]] float4x4 model;
    [[vk::location(2)]] float4x4 view;
};

struct DSOutput
{
    float4 position : SV_Position;
    [[vk::location(0)]] float4 color : COLOR0;
};

cbuffer ubo : register(b0, space0)
{
    UniformBuffer ubo;
}

Texture2D texHeightMap : register(t0, space1);
SamplerState sampHeightMap : register(s0, space1);

DSOutput main(ConstHSOutput inChs,float2 tessCoord:SV_DomainLocation,const OutputPatch<HSOutput,4> patch)
{
    DSOutput dsOut;
    float u = tessCoord.x, v = tessCoord.y;
    float2 texUp = lerp(patch[0].texCoord, patch[1].texCoord, u);
    float2 texDown = lerp(patch[2].texCoord, patch[3].texCoord, u);
    float2 texInt = lerp(texUp, texDown, v);
    
    float4 posUp = lerp(patch[0].position, patch[1].position, u);
    float4 posDown = lerp(patch[2].position, patch[3].position, u);
    float4 posInt = lerp(posUp, posDown, v);
    
    float h = texHeightMap.SampleLevel(sampHeightMap, texInt, 1).r * 0.9 + 0.1;
    posInt.y = h * 5.0;
    
    dsOut.position = mul(ubo.proj, mul(ubo.view, mul(ubo.model, float4(posInt.xyz, 1.0))));
    dsOut.color = float4(h,h,h,1.0);
    return dsOut;
}

