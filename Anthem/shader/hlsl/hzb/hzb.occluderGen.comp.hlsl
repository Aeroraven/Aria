AppendStructuredBuffer<float4> occluderPos : register(u0);

[numthreads(8, 8, 8)]
void main(uint3 invId:SV_DispatchThreadID){
    if(invId.x==0 && invId.y==0 && invId.z==0){
        occluderPos.Append(float4(5,0,0,0));
    }
}