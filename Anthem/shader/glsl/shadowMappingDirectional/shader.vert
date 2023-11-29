#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;

layout (location = 0) out vec4 outPosition;

layout (set = 0,binding = 0) uniform UniformBufferObject{
    mat4 projMat;
    mat4 viewMat;
    mat4 modelMat;
    mat4 normalMat;
}ubo;

void main(){
    outPosition = ubo.projMat * ubo.viewMat * ubo.modelMat * vec4(inPosition.xyz,1.0);
    gl_Position = outPosition;
}