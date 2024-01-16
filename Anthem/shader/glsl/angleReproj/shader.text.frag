#version 450

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec2 inTex;
layout(location = 0) out vec4 outColor;

layout(set = 0,binding = 0) uniform UniformBufferObject{
    mat4 projMat;
    mat4 viewMat;
    mat4 localMat;
}ubo;

layout(set = 1, binding = 0) uniform sampler2D tex;

void main(){
    outColor = inColor;
    outColor = vec4(0.0,1.0,0.0,texture(tex,inTex.xy).r);
}