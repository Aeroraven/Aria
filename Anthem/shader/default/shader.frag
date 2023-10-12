#version 450

// Code from https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Shader_modules


layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}