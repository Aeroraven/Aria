#version 300 es
uniform mat4 uModel;
uniform mat4 uProj;
in vec4 aPos;
in vec2 aTex;
out vec4 vModelPos;
out vec2 vTex;

void main(){
    vTex = aTex;
    vModelPos = uModel * aPos;
    gl_Position = uProj * vModelPos;
}