#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;

layout (set = 0,binding = 0) uniform UniformBufferObject{
    mat4 projMat;
    mat4 viewMat;
    mat4 modelMat;
    mat4 normalMat;
    mat4 lightPovProj;
    mat4 lightPovView;
    vec4 lightDir;
        vec4 shadowMapParams;
}ubo;

void main(){
    outPosition = ubo.modelMat * vec4(inPosition.xyz,1.0);
    outNormal = (ubo.normalMat * vec4(inNormal,0.0));
    gl_Position = ubo.projMat * ubo.viewMat * outPosition;
}