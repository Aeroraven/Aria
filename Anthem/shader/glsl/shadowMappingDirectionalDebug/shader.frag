#version 450

layout(set = 0, binding = 0) uniform sampler2D texSampler;
layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;

void main() {
    vec4 col = texture(texSampler, fragTexCoord);
    outColor = col;
}