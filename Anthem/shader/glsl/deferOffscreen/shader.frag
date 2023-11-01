#version 450

layout(set = 1, binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec4 fragPosition;

layout(location = 0) out vec4 outAlbedo;
layout(location = 1) out vec4 outPosition;
layout(location = 2) out vec4 outNormal;


void main() {
    vec4 col = texture(texSampler, fragTexCoord);
    outAlbedo = col;
    outPosition = fragPosition;
    outNormal = vec4(fragNormal,1.0);
}