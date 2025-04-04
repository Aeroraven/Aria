#version 450

layout(location = 0) in vec4 inColor;
layout(location = 0) out vec4 outColor;
layout(location = 1) in vec4 inRawData;
layout(set = 1, binding = 0) uniform sampler2D tex;

void main(){
    vec4 red = vec4(1.0,0.0,0.0,1.0);
    vec4 blue = vec4(0.0,0.0,1.0,1.0);
    vec4 white = vec4(1.0,1.0,1.0,1.0);
    if(inRawData.y<0.0){
        outColor = mix(white,blue,-inRawData.y);
    }else{
        outColor = mix(white,red,inRawData.y);
    }
    outColor = texture(tex,(vec2(inRawData.y)+1.0)/2.0);

    //outColor=vec4((inRawData.y+1.0)/2.0);

}