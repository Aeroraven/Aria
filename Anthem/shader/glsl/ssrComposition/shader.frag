#version 450

layout(location = 0) in vec2 inTex;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D positionTex;
layout(set = 1, binding = 0) uniform sampler2D colorTex;
layout(set = 2, binding = 0) uniform sampler2D normalTex;
layout(set = 3, binding = 0) uniform sampler2D specularTex;
layout(set = 4, binding = 0) uniform UniformBufferObject{
    mat4 projMat;
    mat4 viewMat;
    mat4 modelMat;
    mat4 normalMat;
    vec4 windowState;
}ubo;

const float maxDist = 100.0;
const float depthAlw = 1.0;

void main(){
    vec2 scrSize = ubo.windowState.xy;
    vec2 texUnit = 1.0/scrSize;

    vec3 vsPosition = texture(positionTex,inTex).rgb;
    vec3 vsNormal = normalize(texture(normalTex,inTex).rgb);
    vec3 vsEyeRay = normalize(vsPosition);
    vec3 vsReflect = reflect(-vsEyeRay,vsNormal);
    vec3 vsColor = texture(colorTex,inTex).rgb;

    float isSpecular = texture(specularTex,inTex).r;


    if(isSpecular < 0.5){
        outColor = vec4(vec3(0.0,0.0,0.0),1.0);
        return;
    }

    vec3 vsAdvanceDir = vsReflect;
    vec3 vsStartPoint = vsPosition + vsAdvanceDir * (2e-3);
    vec3 vsEndPoint = vsPosition + vsAdvanceDir * maxDist;
    
    vec4 ssStartPoint = ubo.projMat * vec4(vsStartPoint,1.0);
    ssStartPoint/=ssStartPoint.w;
    vec4 ssEndPoint = ubo.projMat * vec4(vsEndPoint,1.0);
    ssEndPoint/=ssEndPoint.w;

    vec2 ssStartPx = (ssStartPoint.xy + 1.0)/2.0;
    ssStartPx = clamp(ssStartPx,vec2(0.0),vec2(1.0));
    vec2 ssEndPx = (ssEndPoint.xy + 1.0)/2.0;
    outColor = vec4(vsReflect,1.0);
    ssEndPx = clamp(ssEndPx,vec2(0.0),vec2(1.0));

    vec2 deltaCoord = (ssEndPx-ssStartPx)/texUnit;
    int deltaSel = int(max(deltaCoord.x,deltaCoord.y));

    return;

    int hit = 0;
    vec4 reflColor = vec4(1.0);
    vec2 reflCoord = vec2(0.0);
    for(int i=0;i<deltaSel;i++){
        float percent = mix(0.0,1.0,float(i)/float(deltaSel));
        vec2 curSsPx = mix(ssStartPx,ssEndPx,percent);
        float curVsDepth = mix(ssStartPoint.z,ssEndPoint.z,percent);
        float curSsDepth = texture(positionTex,curSsPx).z;
        
        float refDepth = curSsDepth - curVsDepth;
        if(refDepth<-1e-3 && abs(refDepth) < depthAlw){
            hit = 1;
            reflColor.rgb = texture(colorTex,curSsPx).rgb;
            reflCoord = curSsPx;
        }
    }

    if(hit==1){
        outColor = vec4(0.0,vec2(reflCoord),1.0);
    }else{
        outColor = vec4(1.0);
    }
}