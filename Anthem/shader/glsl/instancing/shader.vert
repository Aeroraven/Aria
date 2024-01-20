#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inInstance;

void main() {
    gl_PointSize = 14.0;
    gl_Position = vec4(inPosition+inInstance,0.5, 1.0);
}