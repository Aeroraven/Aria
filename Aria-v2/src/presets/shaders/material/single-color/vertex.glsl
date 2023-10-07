#version 300 es
uniform mat4 uModel;
uniform mat4 uProj;
uniform mat4 uLocal;
in vec4 aPos;
out vec4 vModelPos;

void main(){
    vModelPos = uModel * uLocal * aPos;
    gl_Position = uProj * vModelPos;
}