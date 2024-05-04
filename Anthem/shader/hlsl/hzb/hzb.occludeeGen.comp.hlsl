#include "../common/random.hlsl"
AppendStructuredBuffer<float4> occludeePos : register(u0,space0);

[numthreads(8, 8, 8)]
void main(uint3 invId:SV_DispatchThreadID){
    const float scale = 1.0f;
    float3 invIdf = float3(invId);
    float3 pos = float3(invIdf.x, invIdf.y, invIdf.z) * scale;
    float randomv = (trivialRandom(pos.xy)+trivialRandom(pos.yz)+trivialRandom(pos.zx))/3.0f;
    if(randomv<0.12f){
        float negX = sign(trivialRandom(pos.yx+pos.zy)-0.5);
        float negY = sign(trivialRandom(pos.xz+pos.zy)-0.5);

        pos =float3(negX*pos.x,negY*pos.y,pos.z);

        occludeePos.Append(float4(pos,1));
    }
}