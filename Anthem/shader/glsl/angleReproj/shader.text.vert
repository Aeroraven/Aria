#version 450

layout(location=0) in vec4 inPosition;
layout(location=1) in vec2 inTex;

layout(set = 0,binding = 0) uniform UniformBufferObject{
    mat4 projMat;
    mat4 viewMat;
    mat4 localMat;
}ubo;
layout(set = 1, binding = 0) uniform sampler2D tex;


layout(location=0) out vec4 outColor;
layout(location=1) out vec2 outTex;

void main(){
    gl_PointSize = 2.0f;
    gl_Position = vec4(inPosition.xyz,1.0);

    outColor = vec4(1.0);
    outTex = inTex;
}