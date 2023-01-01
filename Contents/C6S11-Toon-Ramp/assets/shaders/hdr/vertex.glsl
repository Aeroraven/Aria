#version 300 es

in vec4 aVert;
in vec2 aTex;

out mediump vec2 vTex;

uniform mat4 uProj;


void main(){
    gl_Position = uProj * aVert;
    vTex = aTex;
}