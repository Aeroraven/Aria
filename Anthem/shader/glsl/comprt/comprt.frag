#version 450

layout(set = 0,binding = 0) uniform sampler2D tex;
layout(location=0) out  vec4 outColor;
layout(location=0) in  vec2 inPos;
void main(){
    vec2 inPosX = inPos * 0.5+0.5;
    outColor = texture(tex,inPosX);
}