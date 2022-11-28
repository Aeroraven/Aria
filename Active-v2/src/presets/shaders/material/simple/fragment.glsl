#version 300 es
precision highp float;
uniform sampler2D uBackground;
uniform vec4 uPresetColor;

//Camera
uniform highp mat4 uModel;

//Light Uniforms
const int MAX_LIGHTS = 256;
uniform int uLightCount;
uniform vec3 uLightPos[MAX_LIGHTS];
uniform vec4 uLightColor[MAX_LIGHTS];
uniform int uLightType[MAX_LIGHTS];

//Varyings
in vec4 vNorm;

//Out
out vec4 fragmentColor;


void main(){
    vec4 diffuse = vec4(0.0);
    for(int i=0;i<uLightCount;i++){
        diffuse += uLightColor[i] * max(0.0, dot(-uModel*normalize(vec4(uLightPos[i],0.0)), normalize(vNorm.xyzw)));
    }
    fragmentColor = vec4(diffuse * uPresetColor);
}