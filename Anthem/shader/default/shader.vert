#version 450

// Code from https://vulkan-tutorial.com/Vertex_buffers/Vertex_input_description

layout(binding = 0) uniform UniformBufferObject {
    vec4 addColor;
    mat4 testMat;
} ubo;
layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    gl_Position = ubo.testMat * vec4(inPosition, 0.0, 1.0);
    fragColor = inColor + ubo.addColor.xyz;
    fragTexCoord = inTexCoord;
}