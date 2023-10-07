#version 300 es
uniform mat4 uModel;
uniform mat4 uProj;
uniform mat4 uViewOrtho;
in vec4 aPos;
in vec2 aTex;

out vec2 vTex;

void main(){
    vTex = aTex;
    gl_Position = uViewOrtho  * aPos;
}