#version 300 es

in vec4 aVert;
in vec2 aTex;

out highp vec2 vTex;
out highp vec3 vPos;

uniform mat4 uViewOrtho;


void main(){
    gl_Position = uViewOrtho * aVert;
    vTex = aTex;
    vPos = gl_Position.xyz;
    
}