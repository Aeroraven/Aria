struct VSOutput{
    float4 position : SV_Position;
    [[vk::location(0)]] float4 normal : POSITION0;
};
struct GSOutput{
    float4 position : SV_Position;
};

[maxvertexcount(6)]
void main(triangle VSOutput vsOut[3], inout TriangleStream<GSOutput> outStream)
{
    float edgeLen[3];
    float2 ndcPos[3];
    float angle[3];
    float extLen[3];
    float2 ndcOff[3];
    float4 newPos[3];
    for (int i = 0; i < 3; i++)
    {
        ndcPos[i] = vsOut[i].position.xy / vsOut[i].position.w;
    }
    for (int i = 0; i < 3; i++)
    {
        float2 v1 = normalize(ndcPos[(i - 1 + 3) % 3] - ndcPos[i]);
        float2 v2 = normalize(ndcPos[(i + 1) % 3] - ndcPos[i]);
        angle[i] = acos(dot(v1, v2));
        extLen[i] = min(0.004, 0.002 / (1e-4 + sin(angle[i] / 2)));
        if (angle[i] < 1e-3)
        {
            extLen[i] = 0;
        }
    }
    edgeLen[0] = length(ndcPos[1] - ndcPos[2]);
    edgeLen[1] = length(ndcPos[2] - ndcPos[0]);
    edgeLen[2] = length(ndcPos[1] - ndcPos[0]);
    float2 incenter = (edgeLen[0] * ndcPos[0] + edgeLen[1] * ndcPos[1] + edgeLen[2] * ndcPos[2]) / (edgeLen[0] + edgeLen[1] + edgeLen[2]);
    
    
    for (int i = 0; i < 3; i++)
    {
        ndcOff[i] = normalize(ndcPos[i] - incenter) * extLen[i];
        newPos[i] = vsOut[i].position;
        newPos[i].xy += ndcOff[i] * vsOut[i].position.w;
        GSOutput gsOut;
        gsOut.position = newPos[i];
        outStream.Append(gsOut);
    }
    
    outStream.RestartStrip();
    for (int i = 0; i < 3; i++)
    {
        GSOutput gsOut;
        gsOut.position = vsOut[i].normal;
        outStream.Append(gsOut);
    }
    outStream.RestartStrip();
}