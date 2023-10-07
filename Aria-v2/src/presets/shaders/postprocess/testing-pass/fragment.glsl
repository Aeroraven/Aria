#version 300 es
precision mediump float;
uniform sampler2D uSourceFrame;
uniform sampler2D uBlendingWeight;
out vec4 fragmentColor;
in vec2 vTex;

void main(){
    fragmentColor = vec4(vec2(vTex),0.0,1.0);   
}