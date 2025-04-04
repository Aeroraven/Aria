Texture2D texSource: register(t0);
SamplerState sampSource: register(s0);
RWTexture2D<float> texDest: register(u0,space1);

[numthreads(1, 1, 1)]
void main(uint3 invId:SV_DispatchThreadID){
    //Copy
    int texSrcH,texSrcW,texSrcLod;
    int2 uv = int2(invId.xy);
    texSource.GetDimensions(0,texSrcW,texSrcH,texSrcLod);
    float4 color = texSource.SampleLevel(sampSource, uv/float2(texSrcW,texSrcH), 0);
    texDest[uv] = color.r;
}