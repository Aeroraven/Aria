#version 300 es
precision mediump float;
uniform sampler2D uSourceFrame[5];
uniform int uSourceCount;
out vec4 fragmentColor;
in vec2 vTex;

void main(){
    vec4 ret = vec4(0.0);
    if(uSourceCount<=1){
        ret += texture(uSourceFrame[0],vTex); 
    }
    if(uSourceCount<=2){
        ret += texture(uSourceFrame[1],vTex); 
    }
    if(uSourceCount<=3){
        ret += texture(uSourceFrame[2],vTex); 
    }
    if(uSourceCount<=4){
        ret += texture(uSourceFrame[3],vTex); 
    }
    fragmentColor = ret; 
}