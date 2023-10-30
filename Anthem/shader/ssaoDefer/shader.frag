#version 450

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec4 fragPosition;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;


void main() {
    outPosition = fragPosition;
    outNormal = vec4(fragNormal,1.0);
}