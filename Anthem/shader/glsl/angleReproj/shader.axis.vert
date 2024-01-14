#version 450

layout(location=0) in vec4 inRandom;
layout(location=1) in vec4 inPosition;

layout(location=0) out vec4 outColor;
void main(){
    gl_PointSize = 100.0f;
    gl_Position = vec4(inPosition.xy,0.5,1.0);

    outColor = vec4(1.0);
}