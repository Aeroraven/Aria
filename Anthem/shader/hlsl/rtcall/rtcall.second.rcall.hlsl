struct CallStruct
{
    float3 color;
};


[shader("callable")]
void main(inout CallStruct data)
{
    data.color = float3(0.0, 1.0, 0.0);
}