#version 450

layout(location = 0) in vec4 inColor;
layout(location = 0) out vec4 outColor;
layout(location = 1) in vec4 inRawData;

void main(){
    outColor = vec4(inColor) * 0.5 + vec4(0.5);
}