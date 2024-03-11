struct VSOutput
{
    float4 position : SV_Position;
    [[vk::location(0)]] float2 texCoord : TEXCOORD0;
};

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

cbuffer ubo : register(b0, space0)
{
    UniformBuffer ubo;
}



ConstHSOutput ConstHSProc(InputPatch<VSOutput, 4> patch, uint InvId : SV_PrimitiveID)
{
    const float MAX_TESS_LVL = 32;
    const float MIN_TESS_LVL = 4;
    
    float dists[4], levels[4];
    for (int i = 0; i < 4; i++)
    {
        float4 vp = mul(ubo.view, mul(ubo.model, patch[i].position));
        float dist = distance(float3(0.0, 0.0, 0.0), vp.xyz);
        dists[i] = clamp(10.0 - 1.0 * dist, 0.0, 10.0) / 10.0;

    }
    for (int i = 0; i < 4; i++)
    {
        levels[i] = lerp(MIN_TESS_LVL, MAX_TESS_LVL, min(dists[(i + 3) % 4], dists[i]));
    }

    
    ConstHSOutput chsOut;
    chsOut.tessLvlInner[0] = max(levels[1], levels[3]);
    chsOut.tessLvlInner[1] = max(levels[0], levels[2]);
    
    chsOut.tessLvlOuter[0] = levels[0];
    chsOut.tessLvlOuter[1] = levels[1];
    chsOut.tessLvlOuter[2] = levels[2];
    chsOut.tessLvlOuter[3] = levels[3];
    
    return chsOut;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(4)]
[patchconstantfunc("ConstHSProc")]
[maxtessfactor(20.0f)]
HSOutput main(InputPatch<VSOutput, 4> patch, uint InvId : SV_OutputControlPointID)
{
    HSOutput hsOut;
    hsOut.position = patch[InvId].position;
    hsOut.texCoord = patch[InvId].texCoord;
    return hsOut;
}