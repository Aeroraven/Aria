struct VSOutput{
    float4 pos: SV_POSITION;
    [[vk::location(0)]] float4 color: COLOR0;
    [[vk::location(1)]] float4 worldPos : POSITION0;
};

struct GSOutput{
    float4 pos: SV_POSITION;
    [[vk::location(0)]] float4 color: COLOR0;
    [[vk::location(1)]] float4 normal : NORMAL0;
};

[maxvertexcount(3)]
void main(triangle VSOutput input[3], inout TriangleStream<GSOutput> triStream){
    float3 edge1 = input[1].worldPos.xyz - input[0].worldPos.xyz;
    float3 edge2 = input[2].worldPos.xyz - input[0].worldPos.xyz;
    float3 normal = normalize(cross(edge1, edge2));

    GSOutput output;
    for(int i=0;i<3;i++){
        output.pos = input[i].pos;
        output.color = input[i].color;
        output.normal = float4(normal, 0);
        triStream.Append(output);
    }
}