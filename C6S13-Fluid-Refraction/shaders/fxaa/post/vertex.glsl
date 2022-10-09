#version 300 es

in vec4 aVert;
in vec2 aTex;

out highp vec2 vTex;

uniform mat4 uViewOrtho;


void main(){
    gl_Position = uViewOrtho * aVert;
    vTex = aTex;
}