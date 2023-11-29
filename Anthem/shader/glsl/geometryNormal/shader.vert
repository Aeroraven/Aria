#version 450

layout(set = 0,binding = 0) uniform UniformBufferObject{
    mat4 projMat;
    mat4 viewMat;
    mat4 modelMat;
    mat4 normalMat;
}ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 outNormal;

void main(){
    gl_Position = ubo.viewMat * ubo.modelMat * vec4(inPosition,1.0);
    outNormal = (ubo.normalMat*vec4(inNormal,0.0)).rgb;
} 