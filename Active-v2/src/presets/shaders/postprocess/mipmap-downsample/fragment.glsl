#version 300 es
precision mediump float;
uniform sampler2D uSourceFrame;
uniform sampler2D uBlendingWeight;
uniform float uLOD;
out vec4 fragmentColor;
in vec2 vTex;

void main(){
    fragmentColor = textureLod(uSourceFrame,vTex,uLOD);   
}