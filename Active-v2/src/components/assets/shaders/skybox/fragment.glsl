#version 300 es
precision mediump float;

in vec4 vTex;
uniform samplerCube uSkybox;

out vec4 fragmentColor;


void main(){
    fragmentColor = texture(uSkybox, vTex.xyz);
}