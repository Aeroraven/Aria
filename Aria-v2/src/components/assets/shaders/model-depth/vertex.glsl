#version 300 es
uniform mat4 uModel;
uniform mat4 uProj;
uniform mat4 uLocal;
in vec4 aPos;
in vec2 aTex;
out vec4 vModelPos;
out vec2 vTex;

void main(){
    vTex = aTex;
    vModelPos = uModel * uLocal * vec4(aPos.xyz,1.0);
    vModelPos = vModelPos / vModelPos.w;
    gl_Position = uProj * vModelPos;
}