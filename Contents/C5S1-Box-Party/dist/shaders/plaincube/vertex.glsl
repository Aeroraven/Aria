#version 300 es
precision lowp float;
in vec4 aVert;
in vec2 aTex;

out vec2 vTex;

uniform mat4 uModel;
uniform mat4 uProj;

void main(){
    gl_Position = uProj * uModel * aVert;
    vTex = aTex;
}