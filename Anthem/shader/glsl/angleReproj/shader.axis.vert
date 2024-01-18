#version 450

layout(location=0) in vec4 inRandom;
layout(location=1) in vec4 inPosition;
layout(location=2) in float inCol;

layout(set = 0,binding = 0) uniform UniformBufferObject{
    mat4 projMat;
    mat4 viewMat;
    mat4 localMat;
}ubo;


layout(location=0) out vec4 outColor;
void main(){
    gl_PointSize = 2.0f;
    gl_Position = ubo.projMat*ubo.viewMat*ubo.localMat*vec4(inPosition.xyz,1.0);
    
    outColor = vec4(vec3(inCol/2.0*1.5),1.0);
}