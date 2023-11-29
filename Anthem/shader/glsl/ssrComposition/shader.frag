#version 450

// Note: This code is still buggy. 
// This SSR implement adopts single depth buffer and runs without RT algos.
// It might produce strange artifacts for non-convex meshes, especially meshes with hollows (like dress)

// Rectification: This code rectifies the incorrect version in Aria(WebGL-based version)
// which uses RT along view space instead of screen space

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

const float maxDist = 40.0;
const float depthAlw = 0.5;

void main(){
    vec2 scrSize = ubo.windowState.xy;
    vec2 texUnit = 1.0/scrSize;

    vec3 vsPosition = texture(positionTex,inTex).rgb;
    vec3 vsNormal = normalize(texture(normalTex,inTex).rgb);
    vec3 vsEyeRay = normalize(vsPosition);
    vec3 vsReflect = reflect(vsEyeRay,vsNormal);
    vec3 vsColor = texture(colorTex,inTex).rgb;

    float isSpecular = texture(specularTex,inTex).r;

    if(isSpecular < 0.5){
        outColor = vec4(vsColor,1.0);
        return;
    }

    vec3 vsAdvanceDir = vsReflect;
    vec3 vsStartPoint = vsPosition + vsAdvanceDir * (2e-1);
    vec3 vsEndPoint = vsPosition + vsAdvanceDir * maxDist;
    
    vec4 ssStartPoint = ubo.projMat * vec4(vsStartPoint,1.0);
    ssStartPoint/=ssStartPoint.w;
    vec4 ssEndPoint = ubo.projMat * vec4(vsEndPoint,1.0);
    ssEndPoint/=ssEndPoint.w;

    vec2 ssStartPx = (ssStartPoint.xy + 1.0)/2.0;
    ssStartPx = clamp(ssStartPx,vec2(0.0),vec2(1.0));
    vec2 ssEndPx = (ssEndPoint.xy + 1.0)/2.0;
    ssEndPx = clamp(ssEndPx,vec2(0.0),vec2(1.0));

    vec2 deltaCoord = abs((ssEndPx-ssStartPx)/texUnit);
    int deltaSel = int(max(deltaCoord.x,deltaCoord.y));

    int hit = 0;
    vec4 reflColor = vec4(1.0);
    vec2 reflCoord = vec2(0.0);
    float dm = 0.0;

    //First Pass
    int hitStep = 0;
    int lastStep = 0;
    for(int i=0;i<deltaSel;i++){
        float percent = float(i)/float(deltaSel);
        vec2 curSsPx = mix(ssStartPx,ssEndPx,percent);
        float curVsDepth = (vsStartPoint.z*vsEndPoint.z)/mix(vsEndPoint.z,vsStartPoint.z,percent);
        float curSsDepth = texture(positionTex,curSsPx).z;
        float refDepth = curSsDepth - curVsDepth;

        if(abs(curSsDepth)<1e-3 || texture(specularTex,curSsPx).r > 0.5){
            continue;
        }

        if(refDepth<depthAlw && abs(refDepth)<depthAlw){
            hit = 1;
            reflColor.rgb = texture(colorTex,curSsPx).rgb;
            reflCoord = curSsPx;
            hitStep = i;
            break;
        }
        lastStep = i;
    }

    // Second Pass
    float fade1 = 0.0;
    float refinedHitPercent = 1.0;
    if(hit==1){
        const int steps = 5;
        float left = float(lastStep)/float(deltaSel);
        float right = float(hitStep)/float(deltaSel);
        
        for(int i=0;i<steps;i++){
            float percent = (left+right)/2;
            refinedHitPercent = percent;
            vec2 curSsPx = mix(ssStartPx,ssEndPx,percent);
            float curVsDepth = (vsStartPoint.z*vsEndPoint.z)/mix(vsEndPoint.z,vsStartPoint.z,percent);
            float curSsDepth = texture(positionTex,curSsPx).z;
            float refDepth = curSsDepth - curVsDepth;

            fade1 = 1.0 - clamp(abs(refDepth) / depthAlw,0.0,1.0);

            if(refDepth<0){
                right=percent;
            }else{
                left=percent;
            }
        }

        //Final Proc
        vec2 finalTexPos =  mix(ssStartPx,ssEndPx,refinedHitPercent);
        reflColor = texture(colorTex,finalTexPos);
    }

    float fade2 = 1.0-max(refinedHitPercent,0.0);
    fade1 = 1.0;
    float vis = fade1 * fade2 * float(hit);

    outColor = vec4(mix(vsColor,reflColor.rgb,vis),1.0);
}