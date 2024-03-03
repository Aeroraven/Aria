#version 450

layout(set = 1, binding = 0) uniform sampler2D tex[5];

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 inTex;

void main() {
    outColor = texture(tex[1],inTex).rgba;
}