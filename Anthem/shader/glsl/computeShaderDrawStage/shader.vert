#version 450

layout(location=0) in vec2 inPosition;
layout(location=1) in vec2 inTex;
layout(location=2) in vec4 inColor;

layout(location=0) out vec4 outColor;
void main(){
    gl_PointSize = 5.0f;
    gl_Position = vec4(inPosition,0.5,1.0);

    outColor = inColor;
}