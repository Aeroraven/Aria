attribute vec4 aVert;
attribute vec4 aColor;
attribute vec2 aTex;
attribute vec3 aNorm;

varying mediump vec4 vColor;
varying mediump vec2 vTex;
varying mediump vec3 vNorm;
varying mediump vec3 vFragPos;


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