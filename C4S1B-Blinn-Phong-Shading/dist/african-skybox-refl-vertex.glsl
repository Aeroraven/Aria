#version 300 es

in vec4 aVert;
in vec4 aColor;
in vec2 aTex;
in vec3 aNorm;

out mediump vec4 vColor;
out mediump vec2 vTex;
out mediump vec3 vNorm;
out mediump vec3 vFragPos;


uniform mat4 uModelInvTrans;
uniform mat4 uModel;
uniform mat4 uProj;


void main(){
    gl_Position = uProj * uModel * aVert;
    vColor = aColor;
    vTex = vec2(aTex.x,1.0-aTex.y);
    vNorm = vec3((uModelInvTrans*vec4(aNorm,0.0)).xyz);
    vFragPos = (uModel * aVert).xyz;
}