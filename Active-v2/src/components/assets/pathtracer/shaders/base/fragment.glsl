#version 300 es
precision mediump float;

in vec2 vRayDirection;
in vec3 vOriginPos;
in vec2 vTex;
out vec4 fragmentColor;

uniform sampler2D uSrcFrame;
uniform float uTime;
uniform float uframes;

int uMaxRayIters = 20;
int uSampleTimes = 20;
float uCurRenderIter = 0.0;

float rNoise = 0.0; 

const float epsf = 1e-6;
const float pi = 3.1415926535897932384626433832795;
const float maxf = 1e20;
const float maxCondf = 1e10;

const int iMaxTracingIters = 20;

struct rts_Ray{
    vec3 origin;
    vec3 direction;
};

float rtf_random3(vec3 uv){
    rNoise += (uTime+uCurRenderIter)*uTime;
    return fract(sin(dot(uv+vec3(rNoise), vec3(78.233+uTime,12.9898, 45.5432-uTime))) * 43758.5453);
}
float rtf_random(float v){
    return rtf_random3(vec3(v)*uTime);
}
vec3 rtf_randomUnitVector(vec3 uv){
    float a = rtf_random3(uv)*2.0*pi;
    float b = rtf_random3(uv)*2.0*pi;
    float z = sin(a);
    float r = sqrt(1.0-z*z);
    float x = r*cos(b);
    float y = r*sin(b);
    return vec3(x,y,z);
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
    vec4 skyColor = vec4(1.8);
    vec4 accumulatedColor = vec4(skyColor);
    int reverseMaxIters = uMaxRayIters-1;

    vec4[iMaxTracingIters] acEmission; //= vec4[iMaxTracingIters](vec4(0.0));
    vec4[iMaxTracingIters] acMaterial; //= vec4[iMaxTracingIters](vec4(0.0));
    vec3[iMaxTracingIters] acHitPoint;// = vec3[iMaxTracingIters](vec3(0.0));
    vec3[iMaxTracingIters] acHitNorm; //= vec3[iMaxTracingIters](vec3(0.0));
    vec3[iMaxTracingIters] acInRayDir;// = vec3[iMaxTracingIters](vec3(0.0));
    vec3[iMaxTracingIters] acOutRayDir; //  = vec3[iMaxTracingIters](vec3(0.0));
    
    for(int i=0;i<uMaxRayIters;i++){
        //Process Intersection
        float advanceDist = 0.0;
        vec3 hitNorm = vec3(0.0);
        int hitMaterial = -1;
        hitTest(r,advanceDist,hitNorm,hitMaterial);
        if(advanceDist>maxCondf){
            reverseMaxIters = i-1;
            accumulatedColor = skyColor;
            //return vec4(0.0,1.0,0.0,1.0);
            break;
        }
        //return vec4(1.0,1.0,0.0,1.0);
        vec3 hitPos = r.origin + r.direction*advanceDist;

        //Process Materials
        acHitPoint[i] = hitPos;
        acHitNorm[i] = hitNorm;
        acInRayDir[i] = r.direction;
        processMaterialTracingPhase(hitMaterial,hitPos,hitNorm,r.direction,acOutRayDir[i],acEmission[i],acMaterial[i]);

        //Next Ray
        r.origin = hitPos;
        r.direction = acOutRayDir[i];
    }
    for(int i=reverseMaxIters;i>=0;i--){
        accumulatedColor = accumulatedColor * acMaterial[i] + acEmission[i];
        accumulatedMaterial = accumulatedMaterial * acMaterial[i];
    }
    return accumulatedColor;
}

void main(){
    vec4 result = vec4(0.0);
    vec3 normalizedRayDir = normalize(vec3(vRayDirection,1.0));
    vec4 beforeResult = texture(uSrcFrame, vTex.st);
    rts_Ray r;
    for(int T=0;T<uSampleTimes;T++){
        r.origin = vec3(vOriginPos);
        r.direction = vec3(normalizedRayDir);
        result += rtf_rayTracing(r)/float(uSampleTimes);
        uCurRenderIter+=1.0;
    }
    fragmentColor = mix(result,beforeResult,vec4(1.0/(uframes+1.0)));
}
