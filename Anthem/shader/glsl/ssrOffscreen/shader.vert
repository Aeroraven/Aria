#version 450

layout(set = 1,binding = 0) uniform UniformBufferObject{
    mat4 projMat;
    mat4 viewMat;
    mat4 modelMat;
    mat4 normalMat;
    vec4 windowState;
}ubo;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTex;
layout(location = 3) in vec4 inMisc;

layout(location = 0) out vec3 outPos;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec2 outTex;
layout(location = 3) out vec4 outMisc;

void main(){
    vec4 viewSpacePosition = ubo.viewMat * ubo.modelMat * vec4(inPos,1.0);
    outPos = viewSpacePosition.xyz;
    outNormal = (ubo.normalMat * vec4(inNormal,0.0)).rgb;
    outTex = inTex;
    outMisc = inMisc;
    gl_Position = ubo.projMat * viewSpacePosition;
}