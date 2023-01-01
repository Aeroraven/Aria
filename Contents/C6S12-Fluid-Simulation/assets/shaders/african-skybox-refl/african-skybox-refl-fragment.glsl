#version 300 es
precision mediump float;

out vec4 fragColor;

in mediump vec4 vColsor;
in mediump vec2 vTex;
in mediump vec3 vNorm;
in mediump vec3 vFragPos;

uniform sampler2D uSampler;
uniform sampler2D uSpecSampler;
uniform samplerCube uSkybox;
uniform vec3 uCamPos;

void main(){
    vec3 inputLight = normalize(vFragPos - uCamPos);
    vec3 reflectLight = reflect(inputLight,normalize(vNorm));
    vec4 texColor = vec4(texture(uSkybox,reflectLight).rgb,1.0);
    fragColor = texColor;
}