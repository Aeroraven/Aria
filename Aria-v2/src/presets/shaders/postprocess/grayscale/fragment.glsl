#version 300 es
precision mediump float;
uniform sampler2D uSourceFrame;
out vec4 fragmentColor;
in vec2 vTex;

void main(){
    vec4 cl = texture(uSourceFrame,vTex);   
    fragmentColor = vec4(vec3(cl.r*0.299+cl.g*0.587+cl.b*0.114),1.0);
}