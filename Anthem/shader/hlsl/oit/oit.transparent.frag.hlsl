struct PSOutput
{
    float4 accu : SV_TARGET0;
    float4 reveal : SV_TARGET1;
};

struct VSOutput
{
    [[vk::location(0)]] float4 color : COLOR0;
};

PSOutput main(VSOutput vsOut)
{
    PSOutput psOut;
    psOut.accu = vsOut.color;
    psOut.reveal = vsOut.color;
    return psOut;
}