#version 300 es

precision mediump float;

in mediump vec2 vTex;
uniform sampler2D uDiffuse;

out vec4 fragmentColor;

void main(){
    fragmentColor = texture(uDiffuse,vTex);   
}