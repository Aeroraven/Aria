#version 300 es
precision highp float;
uniform highp mat4 uModel;
uniform mat4 uModelIT;
uniform mat4 uProj;
uniform mat4 uLocal;
in vec4 aPos;
in vec4 aNormal;
out vec4 vNorm;
out vec4 vModelPos;
out vec4 vLocalPos;
void main(){
    vNorm = uModelIT * uLocal * aNormal;
    //vNorm /= vNorm.w;
    vLocalPos = uLocal * vec4(aPos.xyz,1.0);
    vLocalPos = vLocalPos / vLocalPos.w;
    vModelPos = uModel * uLocal * vec4(aPos.xyz,1.0);
    vModelPos /= vModelPos.w;
    gl_Position = uProj * vModelPos;
}