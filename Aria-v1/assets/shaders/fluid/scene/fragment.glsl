#version 300 es
precision highp float;
in highp vec4 vColor;
in highp vec2 vTex;
in highp vec3 vNorm;
in highp vec4 vFragPos;

uniform sampler2D uDiffuse;
uniform sampler2D uSpecular;
uniform vec3 uCamPos;
uniform mat4 uModel;
uniform mat4 uShadowOrtho;
uniform mat4 uLightModel[20];

out vec4 fragmentColor;

void main(){
    vec4 vPosC = uShadowOrtho * uLightModel[0] * vFragPos;
    vPosC.x*=0.15;
    vPosC.y*=0.15;
    vPosC = (vPosC+1.0)*0.5;

    vec3 texColor = pow(texture(uDiffuse, vTex).rgb, vec3(2.2));
    vec3 light = (uModel * vec4(0.0,1.0,0.0,0.0)).xyz;
    float ambient = 0.25;
    float diffuse = max(dot(vNorm,light),0.0);
    vec3 mixedColor = texColor * (ambient+diffuse);
    fragmentColor =  vec4(pow(mixedColor,vec3(1.0/2.2)),1.0); 
}