struct VSOutput
{
    float4 position : SV_Position;
};
struct Camera
{
    float4x4 proj;
    float4x4 view;
    float4x4 model;
};

ConstantBuffer<Camera> cam[6] : register(b0, space0);

struct GSOutput
{
    float4 position : SV_Position;
    uint slice : SV_RenderTargetArrayIndex;
    [[vk::location(0)]] float4 orgPosition : POSITION0;
};

[maxvertexcount(18)]
void main(triangle VSOutput vsOut[3], inout TriangleStream<GSOutput> gsOutStream)
{
    for (uint k = 0; k < 6; k++)
    {
        for (int i = 0; i < 3; i++)
        {
            GSOutput gsOut;
            gsOut.slice = k;
            gsOut.orgPosition = vsOut[i].position;
            gsOut.position = mul(cam[k].proj, mul(cam[k].view, mul(cam[k].model, vsOut[i].position)));
            gsOutStream.Append(gsOut);
        }
        gsOutStream.RestartStrip();
    }
}