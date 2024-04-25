struct VSOutput
{
    float4 position : SV_Position;
    [[vk::location(0)]] float4 normal : NORMAL0;
    [[vk::location(1)]] float4 rawPosition : POSITION0;
    [[vk::location(2)]] float4 tangent : TANGENT0;
    [[vk::location(3)]] float4 instancePos : POSITION1;
    [[vk::location(4)]] float4 texIndices : POSITION2;
    [[vk::location(5)]] float4 uv : TEXCOORD0;
};

Texture2D tex[5] : register(t0, space0);
SamplerState samp[5] : register(s0, space0);

void main(VSOutput vsOut)
{
    int baseColorIndex = int(vsOut.texIndices.r);
    if (tex[baseColorIndex].Sample(samp[baseColorIndex], vsOut.uv.xy).a < 0.5)
    {
        discard;
    }
}
