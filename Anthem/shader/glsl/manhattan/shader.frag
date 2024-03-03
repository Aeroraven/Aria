#version 450

layout(location = 0) in vec4 inColor;
layout(location = 0) out vec4 outColor;
layout(location = 1) in vec4 inRawData;

void main(){
    outColor = pow(vec4(inColor),vec4(1.0/2.2));
}