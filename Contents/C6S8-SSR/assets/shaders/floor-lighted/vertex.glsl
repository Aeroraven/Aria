#version 300 es

in vec2 aTex;
in vec4 aPos;
in vec3 aNorm;

uniform mat4 uModelInvTrans;
uniform mat4 uModel;
uniform mat4 uProj;

out mediump vec2 vTex;
out mediump vec3 vNorm;
out mediump vec3 vFragPos;


void main(){
    gl_Position = uProj * uModel * aPos;
    vTex = vec2(aTex.x,1.0-aTex.y);
    vNorm = vec3((uModelInvTrans*vec4(aNorm,0.0)).xyz);
    vFragPos = (uModel * aPos).xyz;
}