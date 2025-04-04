#version 450

layout(set = 1, binding = 0) uniform sampler2D tex[5];

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec3 inTex;

void main() {
    if(inTex.z<0.1){
        outColor.xyz = texture(tex[0],inTex.xy).rgb;
    }else if(inTex.z<1.1){
        outColor.xyz = texture(tex[1],inTex.xy).rgb;
    }else if(inTex.z<2.1){
        outColor.xyz = texture(tex[2],inTex.xy).rgb;
    }else if(inTex.z<3.1){
        outColor.xyz = texture(tex[3],inTex.xy).rgb;
    }else if(inTex.z<4.1){
        outColor.xyz = texture(tex[4],inTex.xy).rgb;
    }
    outColor.w = 1.0;
}