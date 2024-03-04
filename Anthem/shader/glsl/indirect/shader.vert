#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 proj;
    mat4 view;
    mat4 local;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inTex;
layout(location = 2) in float inTx;

layout(location = 0) out vec3 outTex;

void main() {
    vec3 pos = (ubo.local * vec4(inPosition,1.0)).xyz + vec3(inTx.x,0.0,0.0);
    gl_Position = ubo.proj * ubo.view * vec4(pos, 1.0);
    outTex = inTex;
}