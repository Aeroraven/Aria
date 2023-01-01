attribute vec4 aVert;
attribute vec4 aColor;
attribute vec2 aTex;

varying mediump vec4 vColor;
varying mediump vec2 vTex;

uniform mat4 uModel;
uniform mat4 uProj;

void main(){
    gl_Position = uProj * uModel * aVert;
    vColor = aColor;
    vTex = aTex;
}