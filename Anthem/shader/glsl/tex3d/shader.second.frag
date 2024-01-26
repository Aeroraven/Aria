#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inProjPos;

layout(binding=0,set=0) uniform Camera{
    mat4 proj;
    mat4 view;
    mat4 local;
    vec4 camPos;
}ubo;
layout(binding=0,set=1) uniform sampler2D backFace;
layout(binding=0,set=2) uniform sampler3D uVTexture;

void debug(){
    //outColor = vec4(vec3((inPos+1.0)/2.0), 1.0);
    outColor = vec4(0.2)+texture(backFace, (inPos.xy+1.0)/2.0);
}

void main(){
    vec3 startPos = inPos.xyz;
    vec4 destPosRaw =  vec4(inProjPos, 1.0);
    vec3 destPos = texture(backFace, destPosRaw.xy*0.5+0.5).xyz;
    destPos = destPos * 2.0 - 1.0;

    vec4 redr = vec4(vec3((inPos+1.0)/2.0), 1.0);

    vec3 dir = destPos - startPos;
    vec3 ndir = normalize(dir);
    float ldir = length(dir);

    float accColor = 0.0;
    const float STEP_LENGTH = 0.005;
    const int STEP_MAX = int(8.0 / STEP_LENGTH);
    const float STEP_ALPHA = 0.002;
    float r = 0.0;
    int i = 0;
    for(;i<STEP_MAX;r+=STEP_LENGTH,i+=1){
        if(r>ldir){
            break;
        }
        vec3 cPos = startPos + r*ndir;
        float dist = (max(sqrt(2.0) - length(cPos),0.0))/sqrt(2.0);
        vec3 rmcPos = cPos*0.5 + vec3(0.5);
        accColor += (1.0-texture(uVTexture,rmcPos).r) * STEP_ALPHA * dist;
    }
    outColor = vec4(vec3(accColor),1.0);
}