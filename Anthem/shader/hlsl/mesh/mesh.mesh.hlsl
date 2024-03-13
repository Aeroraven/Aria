struct VertexOutput
{
    float4 position : SV_Position;
    float4 color : COLOR0;
};

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void main(out indices uint3 idx[1], out vertices VertexOutput verts[3], uint3 thIndex : SV_DispatchThreadID)
{
    float offx[3] = { 0.0, -0.5, 0.5 };
    SetMeshOutputCounts(3, 1);
    verts[0].position = float4(-0.2 + offx[thIndex.x], 0.2, 0.5, 1.0);
    verts[1].position = float4(0.2 + offx[thIndex.x], 0.2, 0.5, 1.0);
    verts[2].position = float4(0.0 + offx[thIndex.x], -0.2, 0.5, 1.0);
    verts[0].color = float4(1.0, 0.0, 0.0, 1.0);
    verts[1].color = float4(1.0, 0.0, 0.0, 1.0);
    verts[2].color = float4(1.0, 0.0, 0.0, 1.0);
    
    idx[0] = uint3(0, 1, 2);
}