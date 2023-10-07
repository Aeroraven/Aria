#version 300 es
uniform mat4 uModel;
uniform mat4 uProj;
in vec4 aPos;
out vec4 vTex;

void main(){
    vTex = aPos;
    gl_Position = uProj * uModel * aPos;
}