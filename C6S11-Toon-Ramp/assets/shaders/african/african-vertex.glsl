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
}