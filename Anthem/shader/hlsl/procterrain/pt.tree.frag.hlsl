struct PSOutput
{
    float4 color : SV_Target0;
    float4 normal : SV_Target1;
    float4 position : SV_Target2;
    float4 tangent : SV_Target3;
};

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

Texture2D tex[5] : register(t0, space1);
SamplerState samp[5] : register(s0, space1);

PSOutput main(VSOutput vsOut)
{
    PSOutput psOut;
    int baseColorIndex = int(vsOut.texIndices.r);
    int normalIndex = int(vsOut.texIndices.g);
    psOut.color = tex[baseColorIndex].Sample(samp[baseColorIndex], vsOut.uv.xy);
    if (psOut.color.a < 0.5)
    {
        discard;
    }
    if(normalIndex == 0)
    {
        psOut.normal = vsOut.normal;
    }
    else
    {
        float3 normal = normalize(vsOut.normal.xyz);
        float3 tangent = normalize(vsOut.tangent.xyz);
        tangent = normalize(tangent - dot(tangent, normal) * normal);
        float3 bitangent = cross(normal, tangent);
        float3x3 tbn = transpose(float3x3(tangent, bitangent, normal));
        float3 normalMap = tex[normalIndex].Sample(samp[normalIndex], vsOut.uv.xy).xyz * 2.0 - 1.0;
    
        psOut.normal = float4(normalize(mul(tbn, normalMap)), 0.0);
    }
    psOut.position = vsOut.rawPosition;
    psOut.tangent = vsOut.tangent;   
    return psOut;
}
