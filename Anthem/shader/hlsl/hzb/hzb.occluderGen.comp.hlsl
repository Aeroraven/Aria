AppendStructuredBuffer<float4> occluderPos : register(u0);

[numthreads(8, 8, 8)]
void main(uint3 invId:SV_DispatchThreadID){
    // Seems the shader is useless
    if(invId.x==0 && invId.y==0 && invId.z==0){
        occluderPos.Append(float4(5.4,0,0,0));
        occluderPos.Append(float4(-6.2,2,0,0));
        occluderPos.Append(float4(17.7,3.5,1,0));
        occluderPos.Append(float4(-21.7,-9.5,1,0));
        occluderPos.Append(float4(-36.7,9.5,1,0));
        occluderPos.Append(float4(32.7,3.5,-5,0));
    }
}