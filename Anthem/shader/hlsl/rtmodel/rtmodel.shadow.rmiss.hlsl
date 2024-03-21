struct Payload
{
    [[vk::location(0)]] float3 hitv;
    [[vk::location(1)]] float shadow;
};

[shader("miss")]
void main(inout Payload p)
{
    p.shadow = 0.0;
}