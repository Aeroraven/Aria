#version 300 es
precision highp float;
uniform sampler2D uBackground;
uniform vec4 uPresetColor;

//Camera
uniform highp mat4 uModel;

//Light Uniforms
const int MAX_LIGHTS = 16;
uniform int uLightCount;
uniform vec3 uLightPos[MAX_LIGHTS];
uniform vec4 uLightColor[MAX_LIGHTS];
uniform int uLightType[MAX_LIGHTS];
uniform int uShadowMapType[MAX_LIGHTS];
uniform sampler2D uShadowMap[MAX_LIGHTS];
uniform samplerCube uShadowMapCube[MAX_LIGHTS];

//Varyings
in vec4 vNorm;
in vec4 vModelPos;
in vec4 vLocalPos;

//Out
out vec4 fragmentColor;


void main(){
    fragmentColor = vec4((vec3(0.5)+0.5*normalize(vNorm.xyz)),1.0);
}