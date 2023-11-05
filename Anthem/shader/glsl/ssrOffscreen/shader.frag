#version 450

layout(set = 0,binding = 0) uniform sampler2D baseColorTex;

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

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outPosition;
layout(location = 3) out vec4 outSpecular;

void main(){
    if(inMisc.y>0.5){
        outColor = vec4(texture(baseColorTex,inTex).rgb,0.0);
    }else{
        outColor = vec4(texture(baseColorTex,inTex).rgb,0.0);
    }
    outNormal = vec4(inNormal,1.0);
    outPosition = vec4(inPos,1.0);
    outSpecular = vec4(inMisc.x);
}