#version 300 es
precision mediump float;
uniform samplerCube uSourceFrame;
uniform sampler2D uBlendingWeight;
out vec4 fragmentColor;
in vec2 vTex;

void main(){
    fragmentColor = texture(uSourceFrame,vTex);   
}