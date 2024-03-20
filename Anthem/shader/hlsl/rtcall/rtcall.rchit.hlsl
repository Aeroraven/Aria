struct Attributes
{
    float2 bary;
};
struct CallStruct
{
    float3 color;
};

struct Payload
{
    [[vk::location(0)]] float3 hitv;
};

[shader("closesthit")]
void main(inout Payload p, in Attributes at)
{
    CallStruct data;
    CallShader(InstanceIndex(), data);
    p.hitv = data.color;

}
