#version 300 es
precision highp float;
uniform sampler2D uSourceFrame;
out vec4 fragmentColor;
in vec2 vTex;

//Blur Parameters
uniform float uSigma;
uniform float sum;
uniform float kernel[900];


void main(){
    int kernelWidth = int(3.0*uSigma);
    int i=0;
    ivec2 texSize = textureSize(uSourceFrame, 0);
    vec2 texOffset = 1.0 / vec2(float(texSize.x),float(texSize.y));
    vec4 cl = vec4(0.0);
    float discSum = 0.0;
    for(int j=-kernelWidth;j<=kernelWidth;j++){
        float tx = vTex.x + texOffset.x*float(j);
        vec4 ncl = texture(uSourceFrame,vec2(tx,vTex.y));
        if(tx<0.0||tx>1.0){
            ncl = texture(uSourceFrame,vTex);
            discSum+=kernel[i]; 
            i+=1;
            continue;
        }
        cl+=ncl*kernel[i]; 
        i+=1;
    }
    vec4 finalColor = cl/(sum-discSum);
    fragmentColor = finalColor;
}