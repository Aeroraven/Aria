#version 300 es
precision lowp float;
in vec2 vTex;

out vec4 fragColor;

uniform sampler2D uDiffuse;

void main(){
    fragColor = texture(uDiffuse,vTex);
}