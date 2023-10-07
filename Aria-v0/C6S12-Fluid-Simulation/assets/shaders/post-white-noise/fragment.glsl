#version 300 es

precision highp float;

in highp vec2 vTex;
in highp vec3 vPos;
uniform sampler2D uDiffuse;

out vec4 fragmentColor;

float randomScalar(vec3 worldPos){
    float d = dot(vec3(114.1919, 514.810, 37.719),sin(worldPos));
    return fract(sin(d)*114514.1919810);
}

void main(){
    fragmentColor = vec4(vec3(randomScalar(vPos)),1.0);
}