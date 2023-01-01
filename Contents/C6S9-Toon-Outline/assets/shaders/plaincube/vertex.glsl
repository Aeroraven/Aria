#version 300 es
precision lowp float;
in vec4 aVert;
in vec2 aTex;
in vec3 aNorm;

out vec2 vTex;
out vec3 vNorm;
out vec3 vFragPos;

uniform mat4 uModel;
uniform mat4 uProj;
uniform mat4 uModelIT;

void main(){
    gl_Position = uProj * uModel * aVert;
    vFragPos =  (uModel * aVert).xyz;
    vTex = aTex;
    vNorm = vec3(uModelIT * vec4(aNorm,0.0));
}