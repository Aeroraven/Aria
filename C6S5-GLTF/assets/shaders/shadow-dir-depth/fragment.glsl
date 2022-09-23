#version 300 es
precision highp float;
in mediump vec4 vPos;
out vec4 fragmentColor;

uniform mat4 uShadowOrtho;
uniform mat4 uModel;

void main(){
    vec4 vPosC = uShadowOrtho * uModel * vPos;
    fragmentColor = vec4((vPosC.z+1.0)*0.5,0.0,0.0,1.0);
}