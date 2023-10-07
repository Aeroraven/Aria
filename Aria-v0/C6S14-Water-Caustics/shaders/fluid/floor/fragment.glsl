#version 300 es
precision highp float;
in mediump vec4 vColor;
in mediump vec2 vTex;
in mediump vec3 vNorm;
in mediump vec4 vFragPos;

uniform sampler2D uDiffuse;
uniform sampler2D uSpecular;
uniform vec3 uCamPos;
uniform mat4 uModel;
uniform mat4 uShadowOrtho;
uniform mat4 uLightModel[20];

out vec4 fragmentColor;

void main(){
    vec3 lightDir = (vec4(0.0,1.0,0.0,0.0)).xyz;
    float diffuse = max(0.0,dot(normalize(vNorm),lightDir));
    fragmentColor = vec4(vec3(1.0,1.0,1.0)*0.3,1.0);
}