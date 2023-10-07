#version 300 es

precision highp float;

in mediump vec2 vTex;
uniform sampler2D uDiffuse;
uniform float uScrWidth;
uniform float uScrHeight;
out vec4 fragmentColor;

void main(){
    const int ks = 5;
    const float ksfc = -5.0;
    const float sigma = 24.0;

    float ksf = ksfc;
    float kn[2*ks+1];
    float total = 0.0;
    for(int i=-ks;i<=ks;i++){
        kn[i+ks] = exp(-pow(ksf,2.0)/2.0/sigma);
        ksf+=1.0;
        total += kn[i+ks];
    }

    vec4 t = texture(uDiffuse,vTex).rgba;
    vec4 a = vec4(0.0);
    ksf = ksfc;
    for(int i=-ks;i<=ks;i++){
        a+=texture(uDiffuse,vec2(clamp(vTex.x+ksf/uScrWidth,0.0,1.0),vTex.y)).rgba * kn[i+ks];
        ksf+=1.0;
    }
    a/=total;
    if(a.a>1.0){
        a.a=1.0;
    }
    fragmentColor = vec4(a);
}