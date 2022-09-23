#version 300 es

precision mediump float;

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

    vec3 t = texture(uDiffuse,vTex).xyz;
    vec3 a = vec3(0.0);
    ksf = ksfc;
    for(int i=-ks;i<=ks;i++){
        a+=texture(uDiffuse,vec2(vTex.x,clamp(vTex.y+ksf/uScrHeight,0.0,1.0))).xyz;
        ksf+=1.0;
    }
    a/=total;
    fragmentColor = vec4(a,1.0);
}