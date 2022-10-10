#version 300 es
precision highp float;
in vec4 aVert;
in vec2 aTex;
in vec3 aNorm;

out highp vec4 vColor;
out highp vec2 vTex;
out highp vec3 vNorm;
out highp vec4 vFragPos;
out highp vec4 vModelPos;


uniform mat4 uModelInvTrans;
uniform mat4 uModel;
uniform mat4 uProj;


void main(){
    vModelPos = uModel * aVert;
    gl_Position = uProj * vModelPos;
    vNorm = (uModel * vec4(normalize(aNorm),0.0)).xyz;
    vTex = aTex;
    vFragPos = aVert;
}