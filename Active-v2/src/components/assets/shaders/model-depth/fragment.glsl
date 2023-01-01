#version 300 es
precision mediump float;
uniform sampler2D uBackground;
out vec4 fragmentColor;
in vec2 vTex;
in vec4 vModelPos;

void main(){
    fragmentColor = vec4(vModelPos.xyz,1.0);
}