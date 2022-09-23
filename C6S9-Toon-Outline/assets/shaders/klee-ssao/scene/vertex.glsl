#version 300 es
precision highp float;
in vec4 aVert;
in vec2 aTex;
in vec3 aNorm;

out mediump vec4 vColor;
out mediump vec2 vTex;
out mediump vec3 vNorm;
out mediump vec4 vFragPos;


uniform mat4 uModelInvTrans;
uniform mat4 uModel;
uniform mat4 uProj;


void main(){
    gl_Position = uProj * uModel * aVert;
    vNorm = (uModel * vec4(aNorm,0.0)).xyz;
    vTex = aTex;
    vFragPos = aVert;
}