struct VertexOutput
{
    [[vk::location(0)]] float4 color : COLOR0;
};

float4 main(VertexOutput vsOut) : SV_Target
{
    return vsOut.color;
}