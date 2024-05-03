struct VSOutput{
    [[vk::location(0)]] float4 color: COLOR0;
};

struct PSOutput{
    float4 color: SV_TARGET0;
};

PSOutput main(VSOutput vsOut){
    PSOutput output;
    output.color = vsOut.color;
    return output;
}