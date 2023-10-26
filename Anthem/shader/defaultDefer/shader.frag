#version 450

layout(set = 1, binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outAlbedo;

void main() {
    vec4 col = textureLod(texSampler, fragTexCoord,2.0);
    vec4 org = vec4(fragColor, 1.0);
    outColor = col;
    outAlbedo = vec4(1.0,0.0,0.0,1.0);
}