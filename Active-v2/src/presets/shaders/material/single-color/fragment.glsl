#version 300 es
precision mediump float;
uniform sampler2D uBackground;
uniform vec4 uPresetColor;
out vec4 fragmentColor;

//Light
uniform int uEnableLight;


void main(){
    fragmentColor = vec4(uPresetColor);
}