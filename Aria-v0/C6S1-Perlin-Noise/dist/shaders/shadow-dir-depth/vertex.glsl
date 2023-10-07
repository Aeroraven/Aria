#version 300 es

in vec4 aVert;
in vec2 aTex;

out mediump vec4 vPos;
out mediump vec2 vTex;

uniform mat4 uShadowOrtho;
uniform mat4 uModel;


void main(){
    gl_Position = uShadowOrtho * uModel * aVert;
    vPos = aVert;
}