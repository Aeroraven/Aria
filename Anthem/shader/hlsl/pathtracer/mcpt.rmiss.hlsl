struct Payload
{
    [[vk::location(0)]] float3 hitv;
    [[vk::location(1)]] uint iter;
    [[vk::location(2)]] uint innerIter;
};

[shader("miss")]
void main(inout Payload p)
{
    p.hitv = float3(0.0, 0.0, 0.0);
}