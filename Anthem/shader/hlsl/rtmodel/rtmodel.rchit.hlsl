struct Attributes
{
    float2 bary;
};

struct Payload
{
    [[vk::location(0)]] float3 hitv;
};

[shader("closesthit")]
void main(inout Payload p, in Attributes at)
{
    p.hitv = float3(1.0, 1.0, 1.0);
}
