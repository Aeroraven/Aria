#version 300 es

precision highp float;

in highp vec2 vTex;
in highp vec3 vPos;
uniform sampler2D uDiffuse;
uniform float uTimestamp;

out vec4 fragmentColor;

const int splitParts = 5;

float randomScalar(vec3 worldPos){
    float d = dot(vec3(114.1919, 514.810, 37.719),sin(worldPos));
    return fract(sin(d)*114514.1919810);
}

vec3 randomVec2(vec3 worldPos){
    float d = dot(vec3(114.1919, 514.810, 37.719),sin(worldPos));
    float f = dot(vec3(88.9464, 36.4364, 37.564),sin(worldPos));
    return 2.0*(vec3(fract(sin(d)*11451.1919810),fract(sin(f)*3110.35),0.5)-0.5);
}

vec3 randomVec3(vec3 worldPos){
    float d = dot(vec3(114.1919, 514.810, 37.719),sin(worldPos));
    float f = dot(vec3(88.9464, 36.4364, 47.564),sin(worldPos));
    float g = dot(vec3(89.6464, 19.4949, 80.386),sin(worldPos));
    return 2.0*(vec3(fract(sin(d)*11451.1919810),fract(sin(f)*3110.35),fract(sin(g)*65472.65472))-0.5);
}

float perlinLerp_t(float x){
    float x2 = x*x;
    float x3 = x2*x;
    return x3*(6.0*x2-15.0*x+10.0);
}

float perlinLerp(float s,float t,float x){
    float p = perlinLerp_t(x);
    return (1.0-p)*s+p*t;
}

float lerp(float s,float t,float x){
    return perlinLerp(s,t,x);
}

float noiseFunc(vec3 worldPos){
    float splitPartsf = float(splitParts);
    vec3 magPos = splitPartsf * worldPos;
    vec3 gridMinPos = floor(magPos)/splitPartsf;
    vec3 gridDeltaPos = vec3(1.0,1.0,1.0)/splitPartsf;
    vec3 gridDist = magPos - floor(magPos);
    
    vec3 p000 = gridMinPos+vec3(0.0,0.0,0.0);
    vec3 p100 = gridMinPos+vec3(gridDeltaPos.x,0.0,0.0);
    vec3 p010 = gridMinPos+vec3(0.0,gridDeltaPos.y,0.0);
    vec3 p110 = gridMinPos+vec3(gridDeltaPos.x,gridDeltaPos.y,0.0);
    vec3 p001 = gridMinPos+vec3(0.0,0.0,gridDeltaPos.z);
    vec3 p101 = gridMinPos+vec3(gridDeltaPos.x,0.0,gridDeltaPos.z);
    vec3 p011 = gridMinPos+vec3(0.0,gridDeltaPos.y,gridDeltaPos.z);
    vec3 p111 = gridMinPos+vec3(gridDeltaPos.x,gridDeltaPos.y,gridDeltaPos.z);

    vec3 dir000 = normalize(worldPos-p000);
    vec3 dir010 = normalize(worldPos-p010);
    vec3 dir100 = normalize(worldPos-p100);
    vec3 dir110 = normalize(worldPos-p110);
    vec3 dir001 = normalize(worldPos-p001);
    vec3 dir011 = normalize(worldPos-p011);
    vec3 dir101 = normalize(worldPos-p101);
    vec3 dir111 = normalize(worldPos-p111);

    vec3 grad000 = normalize(randomVec3(p000));
    vec3 grad010 = normalize(randomVec3(p010));
    vec3 grad100 = normalize(randomVec3(p100));
    vec3 grad110 = normalize(randomVec3(p110));
    vec3 grad001 = normalize(randomVec3(p001));
    vec3 grad011 = normalize(randomVec3(p011));
    vec3 grad101 = normalize(randomVec3(p101));
    vec3 grad111 = normalize(randomVec3(p111));
    
    float noise000 = dot(dir000,grad000);
    float noise100 = dot(dir100,grad100);
    float noise010 = dot(dir010,grad010);
    float noise110 = dot(dir110,grad110);
    float noise001 = dot(dir001,grad001);
    float noise101 = dot(dir101,grad101);
    float noise011 = dot(dir011,grad011);
    float noise111 = dot(dir111,grad111);

    float noiseXIntp00 = lerp(noise000,noise100,gridDist.x);
    float noiseXIntp10 = lerp(noise010,noise110,gridDist.x);
    float noiseXIntp01 = lerp(noise001,noise101,gridDist.x);
    float noiseXIntp11 = lerp(noise011,noise111,gridDist.x);

    float noiseYIntp0 = lerp(noiseXIntp00,noiseXIntp10,gridDist.y);
    float noiseYIntp1 = lerp(noiseXIntp01,noiseXIntp11,gridDist.y);

    float noiseZIntp = lerp(noiseYIntp0,noiseYIntp1,gridDist.z);

    return (noiseZIntp+1.0)/2.0;
}

float fractalNoiseFunc(vec3 worldPos){
    float freq = 1.0;
    float ampl = 1.0;
    float res = 0.0;
    float sampl = 0.0;
    for(int i=0;i<5;i=i+1){
        res+=noiseFunc(worldPos*freq)*ampl;
        freq = freq*2.0;
        sampl += ampl;
        ampl = ampl*0.5;
    }
    return res/sampl;
}

void main(){
    fragmentColor = vec4(vec3(fractalNoiseFunc(vec3(vPos.xy,uTimestamp*0.00001))),1.0);
}