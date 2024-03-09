#version 450

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec4 inVel;

layout(push_constant, std140) uniform Ubo{
    mat4 proj;
    mat4 view;
    mat4 model;
};

void main() {
    outColor = vec4(inVel.xy,1.0, 1.0f);
}