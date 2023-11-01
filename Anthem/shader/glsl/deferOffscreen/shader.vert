#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 projMat;
    mat4 viewMat;
    mat4 modelMat;
    mat4 normalMat;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec4 fragPosition;


void main() {
    fragPosition = ubo.modelMat* vec4(inPosition, 1.0);
    gl_Position = ubo.projMat * ubo.viewMat * fragPosition;
    fragNormal = (ubo.normalMat * vec4(inNormal,0.0)).xyz;
    fragTexCoord = inTexCoord;
}