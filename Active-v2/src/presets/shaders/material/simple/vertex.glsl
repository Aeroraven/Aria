#version 300 es
precision highp float;
uniform highp mat4 uModel;
uniform mat4 uProj;
in vec4 aPos;
in vec4 aNormal;
out vec4 vNorm;
out vec4 vModelPos;

void main(){
    vNorm = uModel * aNormal;
    vModelPos = uModel * aPos;
    gl_Position = uProj * vModelPos;
}