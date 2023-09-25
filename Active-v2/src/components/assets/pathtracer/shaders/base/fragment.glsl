#version 300 es
precision mediump float;

in vec2 vRayDirection;
in vec3 vOriginPos;
out vec4 fragmentColor;

const float epsf = 1e-6;
const float pi = 3.1415926535897932384626433832795;

struct rts_Ray{
    vec3 origin;
    vec3 direction;
};

float rtf_random3(vec3 uv){
    return fract(sin(dot(uv, vec3(12.9898, 78.233, 45.5432))) * 43758.5453);
}
float rtf_random(float v){
    return rtf_random3(vec3(v));
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

vec4 hitTest(rts_Ray ray){
    float bestT = 1e80;
    int bestMaterial = -1;
    vec3 bestNorm = vec3(0.0);

    //!@asuPathTracerHitTestInjection()

    if(bestT<1e10){
        return vec4(0.3,0.0,0.0,1.0);
    }else{
        return vec4(0.0,0.0,0.0,1.0);
    }
    
}

void main(){
    vec3 normalizedRayDir = normalize(vec3(vRayDirection,1.0));
    rts_Ray r;
    r.origin = vec3(vOriginPos);
    r.direction = vec3(normalizedRayDir);
    fragmentColor = hitTest(r);
}
