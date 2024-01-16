#version 450

layout(location=0) in vec4 inRandom;
layout(location=1) in vec4 inPosition;

layout(set = 0,binding = 0) uniform UniformBufferObject{
    mat4 projMat;
    mat4 viewMat;
    mat4 localMat;
}ubo;


layout(location=0) out vec4 outColor;
layout(location=1) out vec4 outRawData;
void main(){
    gl_PointSize = 2.0f;
    gl_Position = ubo.projMat*ubo.viewMat*ubo.localMat*vec4(inPosition.xyz,1.0);
    outRawData = vec4(inPosition.xyz,1.0);
    outColor = vec4(1.0);
}