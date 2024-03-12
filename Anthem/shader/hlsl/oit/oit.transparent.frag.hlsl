struct PSOutput
{
    float4 accu : SV_TARGET0;
    float4 reveal : SV_TARGET1;
};

struct VSOutput
{
    [[vk::location(0)]] float4 color : COLOR0;
    [[vk::location(1)]] float posz : POSITION0;
};

PSOutput main(VSOutput vsOut)
{
    PSOutput psOut;
    
    float4 color = vsOut.color;
    float z = vsOut.posz;
    float weight = clamp(pow(min(1.0, color.a * 10.0) + 0.01, 3.0) * 1e8 *
                         pow(1.0 - z * 0.9, 3.0), 1e-2, 3e3);
    float4 accum = float4(color.rgb * color.a, color.a) * weight;
    
    psOut.accu = accum;
    psOut.reveal = float4(color.a, color.a, color.a, color.a);
    return psOut;
}