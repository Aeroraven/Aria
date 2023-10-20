#version 450

// Code from https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Shader_modules


layout(set = 1, binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 col = texture(texSampler, fragTexCoord);
    vec4 org = vec4(fragColor, 1.0);
    outColor = col * 0.008;
}