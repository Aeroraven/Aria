#version 300 es
precision highp float;

in vec4 aVert;
in vec3 aNorm;

out vec3 vLocalPos;

uniform mat4 uModel;
uniform mat4 uProj;
uniform mat4 uModelIT;
uniform mat4 uShadowOrtho;

void main(){
    vec4 vert = aVert;
    vert.w = 1.0;
    vec4 locp = uProj * uModel * vert;
    vec4 tocp = vert;
    vLocalPos = tocp.xyz;
    gl_Position = locp;
}