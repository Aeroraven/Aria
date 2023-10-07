#version 300 es
precision highp float;
out vec4 fragColor;

in vec3 vTex;

uniform samplerCube uSkybox;

void main(){
    fragColor = texture(uSkybox,vTex);
    //fragColor = vec4(1.0,0.0,0.0,1.0);
}