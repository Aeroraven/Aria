#version 300 es
precision mediump float;
uniform sampler2D uBackground;
out vec4 fragmentColor;
in vec2 vTex;

void main(){
    vec2 sTex = vTex;
    sTex.y = vTex.y;
    fragmentColor = texture(uBackground,sTex);   
}