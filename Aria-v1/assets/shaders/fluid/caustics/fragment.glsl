#version 300 es
precision highp float;

in highp vec4 vColor;
in highp vec2 vTex;
in highp vec3 vNorm;
in highp vec4 vFragPos;
in highp vec4 vModelPos;
in highp vec3 vOldPos;
in highp vec3 vNewPos;

uniform sampler2D uDiffuse; //Color
uniform sampler2D uSpecular; //Pos
uniform vec3 uCamPos;
uniform mat4 uModel;
uniform mat4 uProj;
uniform mat4 uShadowOrtho;
uniform mat4 uLightModel[20];

out vec4 fragmentColor;

void main(){
    vec3 op = vOldPos;
    vec3 np = vNewPos;
    float oldSize = length(dFdx(op)) * length(dFdy(op));
    float newSize = length(dFdx(np)) * length(dFdy(np));
    fragmentColor = vec4(vec3(oldSize/newSize)*0.1,1.0);
}