struct Payload
{
    [[vk::location(0)]] float3 hitv;
    [[vk::location(1)]] uint iter;
    [[vk::location(2)]] uint innerIter;
};

[shader("miss")]
void main(inout Payload p)
{
    float light = 0.0;
    p.hitv = float3(light, light, light);
}