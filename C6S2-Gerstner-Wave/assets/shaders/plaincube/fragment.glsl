#version 300 es
precision lowp float;
in vec2 vTex;
in vec3 vNorm;
in vec3 vFragPos;

out vec4 fragColor;

uniform sampler2D uDiffuse;
uniform vec3 uLightPos[2];
uniform vec3 uLightColor[2];
uniform mat4 uModel;

void main(){
    vec4 lightcol = vec4(uLightColor[0],1.0);
    vec3 lightpos = (uModel*vec4(uLightPos[0],1.0)).xyz;
    vec4 texcol = texture(uDiffuse,vec2(vTex.s,vTex.t));

    //Diffuse
    float diffuseStrength = 1.0;
    vec3 norm = normalize(vNorm);
    vec3 lightDir = normalize(lightpos-vFragPos);
    float diffusePower = dot(norm,lightDir);
    vec4 diffusePart = lightcol * diffuseStrength * diffusePower;

    fragColor =  texcol * diffusePart;
}