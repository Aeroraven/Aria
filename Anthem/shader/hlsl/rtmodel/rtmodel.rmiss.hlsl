struct Payload
{
    [[vk::location(0)]] float3 hitv;
};

[shader("miss")]
void main(inout Payload p)
{
    p.hitv = float3(0.0, 0.0, 0.0);
}