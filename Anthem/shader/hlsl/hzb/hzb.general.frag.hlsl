struct GSOutput{
    [[vk::location(0)]] float4 color: COLOR0;
    [[vk::location(1)]] float4 normal : NORMAL0;
};
struct PSOutput{
    float4 color: SV_TARGET0;
};

float4 gamma(float4 color){
    return pow(color, 1.0/2.2);
}

PSOutput main(GSOutput gsOut){
    PSOutput output;
    float3 lightDir = normalize(float3(-1, -1, 1));
    float diffuse = max(0, dot(gsOut.normal.xyz, lightDir));
    float ambient = 0.1;
    output.color = float4(gsOut.color.rgb * (diffuse + ambient), 1);
    output.color = gamma(output.color);
    
    return output;
}