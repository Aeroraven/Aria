#version 450

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inVelocity;
layout(location = 2) in vec4 inLastPos;
layout(location = 3) in vec4 inNormal;
layout(location = 4) in float inTime;

layout(push_constant, std140) uniform Ubo{
    mat4 proj;
    mat4 view;
    mat4 model;
};
layout(location = 0) out vec4 outNorm;
void main() {
    gl_Position = proj*view*model*vec4(inPosition.xyz, 1.0);
    outNorm = inNormal;
}