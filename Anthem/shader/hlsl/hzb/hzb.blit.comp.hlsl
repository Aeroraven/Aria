RWTexture2D<float> blitSrc : register(u0);
RWTexture2D<float> blitDst : register(u0,space1);

[numthreads(1, 1, 1)]
void main(uint3 invId:SV_DispatchThreadID){
    //Max Pooling, window size=(2,2)
    int texSrcH,texSrcW;
    int texDstH,texDstW;
    int2 uv = int2(invId.xy);
    blitSrc.GetDimensions(texSrcW,texSrcH);
    blitDst.GetDimensions(texDstW,texDstH);

    float maxVal = 0;
    for(int i=0;i<2;i++){
        for(int j=0;j<2;j++){
            int2 srcUv = uv*2 + int2(i,j);
            if(srcUv.x<texSrcW && srcUv.y<texSrcH){
                maxVal = max(maxVal, blitSrc[srcUv]);
            }
        }
    }
    blitDst[uv] = maxVal;
}