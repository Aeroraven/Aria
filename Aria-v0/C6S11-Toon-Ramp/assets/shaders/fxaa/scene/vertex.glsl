#version 300 es
precision highp float;
in vec4 aVert;
in vec2 aTex;
in vec3 aNorm;
in vec3 aTangent;
in vec3 aBitangent;

out vec2 vTex;
out vec3 vNorm;
out vec3 vFragPos;
out vec4 vWorldPos;
out vec3 vTangent;
out vec3 vBitangent;


uniform mat4 uModel;
uniform mat4 uProj;
uniform vec3 uInstPos[100];

void main(){
    vec4 vert = (aVert+vec4(uInstPos[gl_InstanceID],0.0));

    gl_Position = uProj * uModel * vert;
    vFragPos =  (uModel*vert).xyz/(uModel*vert).w;
    vTex = aTex;

    vNorm = (uModel * vec4(aNorm,0.0)).xyz;
    vTangent = (uModel * vec4(aTangent,0.0)).xyz;
    vBitangent = (uModel * vec4(aBitangent,0.0)).xyz;

    vWorldPos = vert;
}