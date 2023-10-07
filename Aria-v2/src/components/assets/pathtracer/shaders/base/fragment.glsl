#version 300 es
precision highp float;

in vec2 vRayDirection;
in vec3 vOriginPos;
in vec2 vTex;
out vec4 fragmentColor;

uniform sampler2D uSrcFrame;
uniform float uTime;
uniform float uframes;

int uMaxRayIters = 20;
int uSampleTimes = 5;
float uCurRenderIter = 0.0;

uvec2 seeds = uvec2(554.0,544.0);

float rNoise = 0.0; 

const float epsf = 1e-6;
const float pi = 3.1415926535897932384626433832795;
const float maxf = 1e20;
const float maxCondf = 1e10;

const int iMaxTracingIters = 20;

vec4 rfEmissionColor = vec4(0.0);
vec4 rfMaterialColor = vec4(1.0,0.0,0.0,1.0);

struct rts_Ray{
    vec3 origin;
    vec3 direction;
};

vec4 rtf_gammaCorrection(vec4 x){
    return pow(x,vec4(1.0/2.2));
}

vec4 rtf_inverseGammaCorrection(vec4 x){
    return pow(x,vec4(2.2));
}

float rtf_random(){
    seeds += uvec2(8);
    uvec2 q = 1103515245U * ( (seeds >> 1U) ^ (seeds.yx) );
    uint  n = 1103515245U * ( (q.x) ^ (q.y >> 3U) );
    return float(n) * (1.0 / float(0xffffffffU));
}
float rtf_random2(){
    uvec2 seedss = uvec2(1);
    uvec2 q = 1103515245U * ( (seedss >> 1U) ^ (seedss.yx) );
    uint  n = 1103515245U * ( (q.x) ^ (q.y >> 3U) );
    return float(n) * (1.0 / float(0xffffffffU));
}
vec3 rtf_randomUnitVector() {
    float up = rtf_random() * 2.0 - 1.0; 
    float over = sqrt( max(0.0, 1.0 - up * up) );
    float around = rtf_random() * 6.28318530717;
    return normalize(vec3(cos(around) * over, up, sin(around) * over));	
}

//!@asuPathTracerDefinitionInjection()

void hitTest(rts_Ray ray,inout float advanceDist,inout vec3 hitNorm,inout int hitMaterial){
    float bestT = maxf;
    int bestMaterial = -1;
    vec3 bestNorm = vec3(0.0);

    //!@asuPathTracerHitTestInjection()

    advanceDist = bestT;
    hitNorm = bestNorm;
    hitMaterial = bestMaterial;
}

void processMaterialTracingPhase(int materialIndex, vec3 hitPos, vec3 hitNorm, vec3 inRayDir, inout vec3 outRayDir, inout vec4 emissionColor,inout vec4 materialColor) {
    //!@asuPathTracerProcessMaterialInjection()
}


vec4 rtf_rayTracing(rts_Ray r){

    vec4 accumulatedMaterial = vec4(0.0);
    vec4 skyColor = vec4(0.9,0.95,1.0,1.0);
    vec4 accumulatedColor = vec4(skyColor);
    int reverseMaxIters = uMaxRayIters-1;

    vec4[iMaxTracingIters] acEmission; 
    vec4[iMaxTracingIters] acMaterial;
    vec3[iMaxTracingIters] acHitPoint;
    vec3[iMaxTracingIters] acHitNorm; 
    vec3[iMaxTracingIters] acInRayDir;
    vec3[iMaxTracingIters] acOutRayDir;
    
    //Source Tracing Phase
    for(int i=0;i<uMaxRayIters;i++){
        //Process Intersection
        float advanceDist = 0.0;
        vec3 hitNorm = vec3(0.0);
        int hitMaterial = -1;
        hitTest(r,advanceDist,hitNorm,hitMaterial);
        if(advanceDist>maxCondf){
            reverseMaxIters = i-1;
            accumulatedColor = skyColor;
            break;
        }
        vec3 hitPos = r.origin + r.direction*advanceDist;

        //Process Materials
        acHitPoint[i] = hitPos;
        acHitNorm[i] = hitNorm;
        acInRayDir[i] = r.direction;
        processMaterialTracingPhase(hitMaterial,hitPos,hitNorm,r.direction,acOutRayDir[i],acEmission[i],acMaterial[i]);

        //Next Ray
        r.origin = hitPos+acOutRayDir[i]*1e-4;
        r.direction = acOutRayDir[i];
        //if(i==1){
            //return vec4(vec3(hitPos.z-11.0),1.0);
        //}
        //return vec4(0.0,1.0,0.0,1.0);
        //return acMaterial[i];
    }

    //Forwarding Phase
    for(int i=reverseMaxIters;i>=0;i--){
        accumulatedColor = accumulatedColor * rtf_inverseGammaCorrection(acMaterial[i]) + rtf_inverseGammaCorrection(acEmission[i]);
        accumulatedMaterial = accumulatedMaterial * rtf_inverseGammaCorrection(acMaterial[i]);
    }
    return accumulatedColor;
}



void main(){
    vec4 result = vec4(0.0);
    vec3 normalizedRayDir = normalize(vec3(vRayDirection,1.0));
    vec4 beforeResult = texture(uSrcFrame, vTex.st);
    
    rts_Ray r;
    for(int T=0;T<uSampleTimes;T++){
        seeds += uvec2(gl_FragCoord)*uvec2(uframes*1114.0)+uvec2(T*514);
        r.origin = vec3(vOriginPos);
        r.direction = vec3(normalizedRayDir);
        result += rtf_rayTracing(r)/float(uSampleTimes);
        uCurRenderIter+=1.0;
    }
    vec4 interpFactor = vec4(1.0/(uframes+1.0));
    if(interpFactor.x<5e-3){
        interpFactor = vec4(0.0);
    }
    fragmentColor = mix(beforeResult,rtf_gammaCorrection(result),interpFactor);
}
