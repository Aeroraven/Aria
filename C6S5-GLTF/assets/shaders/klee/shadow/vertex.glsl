#version 300 es

in vec4 aVert;
in vec2 aTex;
in vec3 aNorm;

out mediump vec4 vPos;
out mediump vec2 vTex;
out mediump vec3 vNorm;

uniform mat4 uShadowOrtho;
uniform mat4 uModel;


void main(){
    vec4 vert = aVert;
    vec4 outputV = uShadowOrtho * uModel * vert;
    outputV = outputV / outputV.w;
    outputV.x*=0.25;
    outputV.y*=0.25;
    gl_Position = outputV;
    vPos = vert;
    vTex = aTex;
    vNorm = aNorm;
}