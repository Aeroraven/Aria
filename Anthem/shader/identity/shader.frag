#version 450

layout(set = 0, binding = 0) uniform sampler2D texSampler;
layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;

void main() {
    vec4 col = texture(texSampler, fragTexCoord);
    if(col.r>1e-3||col.g>1e-3||col.b>1e-3){
        outColor = vec4(vec3(0.0),1.0);
    }else{
        outColor = vec4(vec3(1.0),1.0);
    }
}