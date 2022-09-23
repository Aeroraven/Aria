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
uniform mat4 uModelIT;

void main(){
    gl_Position = uProj * uModel * aVert;
    vFragPos =  (uModel * aVert).xyz;
    vTex = aTex;
    vNorm = vec3(uModel * vec4(aNorm,0.0));
    vTangent = vec3((uModel*vec4(aTangent,0.0)).xyz);
    vBitangent = vec3((uModel*vec4(aBitangent,0.0)).xyz);
    vWorldPos = aVert;
}