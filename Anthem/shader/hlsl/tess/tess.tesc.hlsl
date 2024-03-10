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

ConstHSOutput ConstHSProc(InputPatch<VSOutput, 4> patch, uint InvId : SV_PrimitiveID)
{
    ConstHSOutput chsOut;
    chsOut.tessLvlInner[0] = 16;
    chsOut.tessLvlInner[1] = 16;
    
    chsOut.tessLvlOuter[0] = 16;
    chsOut.tessLvlOuter[1] = 16;
    chsOut.tessLvlOuter[2] = 16;
    chsOut.tessLvlOuter[3] = 16;
    
    return chsOut;
}

[domain("quad")]
[outputtopology("triangle_ccw")]
[partitioning("integer")]
[patchconstantfunc("ConstHSProc")]
[outputcontrolpoints(4)]
[maxtessfactor(20.0f)]
HSOutput main(InputPatch<VSOutput, 4> patch, uint InvId : SV_OutputControlPointID)
{
    HSOutput hsOut;
    hsOut.position = patch[InvId].position;
    hsOut.texCoord = patch[InvId].texCoord;
    return hsOut;
}