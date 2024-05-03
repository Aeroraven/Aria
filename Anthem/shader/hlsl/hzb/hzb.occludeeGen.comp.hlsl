AppendStructuredBuffer<float4> occludeePos : register(u0);

[numthreads(8, 8, 8)]
void main(uint3 invId:SV_DispatchThreadID){
    if(invId.x==0 && invId.y==0 && invId.z==0){
        occludeePos.Append(float4(10,0,0,0));
    }
}