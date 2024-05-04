struct VSInput{
    [[vk::location(0)]] float4 pos: POSITION0;
    [[vk::location(1)]] float4 color: COLOR0;
    [[vk::location(2)]] float4 instPos: POSITION1;
};

struct Camera{
    float4x4 mvp;
};

struct VSOutput{
    float4 pos: SV_POSITION;
    [[vk::location(0)]] float4 color: COLOR0;
    [[vk::location(1)]] float4 worldPos : POSITION0;
};

ConstantBuffer<Camera> camera: register(b0);

VSOutput main(VSInput input){
    VSOutput output;
    output.pos = mul(camera.mvp, input.pos+float4(input.instPos.xyz, 0));
    output.color = input.color;
    output.worldPos = input.instPos;
    return output;
}
