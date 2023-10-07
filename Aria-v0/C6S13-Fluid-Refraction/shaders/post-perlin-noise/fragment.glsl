#version 300 es

precision highp float;

in highp vec2 vTex;
in highp vec3 vPos;
uniform sampler2D uDiffuse;

out vec4 fragmentColor;

const int splitParts = 8;

float randomScalar(vec3 worldPos){
    float d = dot(vec3(114.1919, 514.810, 37.719),sin(worldPos));
    return fract(sin(d)*114514.1919810);
}

vec3 randomVec2(vec3 worldPos){
    float d = dot(vec3(114.1919, 514.810, 37.719),sin(worldPos));
    float f = dot(vec3(88.9464, 36.4364, 37.564),sin(worldPos));
    return 2.0*(vec3(fract(sin(d)*11451.1919810),fract(sin(f)*3110.35),0.5)-0.5);
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
    vec3 gridDeltaPos = vec3(1.0,1.0,0.0)/splitPartsf;
    vec3 gridDist = magPos - floor(magPos);
    
    vec3 p00 = gridMinPos+vec3(0.0,0.0,0.0);
    vec3 p10 = gridMinPos+vec3(gridDeltaPos.x,0.0,0.0);
    vec3 p01 = gridMinPos+vec3(0.0,gridDeltaPos.y,0.0);
    vec3 p11 = gridMinPos+vec3(gridDeltaPos.x,gridDeltaPos.y,0.0);

    vec3 dir00 = normalize(worldPos-p00);
    vec3 dir01 = normalize(worldPos-p01);
    vec3 dir10 = normalize(worldPos-p10);
    vec3 dir11 = normalize(worldPos-p11);

    vec3 grad00 = normalize(randomVec2(p00));
    vec3 grad01 = normalize(randomVec2(p01));
    vec3 grad10 = normalize(randomVec2(p10));
    vec3 grad11 = normalize(randomVec2(p11));
    
    float noise00 = dot(dir00,grad00);
    float noise10 = dot(dir10,grad10);
    float noise01 = dot(dir01,grad01);
    float noise11 = dot(dir11,grad11);

    float noiseXIntp0 = lerp(noise00,noise10,gridDist.x);
    float noiseXIntp1 = lerp(noise01,noise11,gridDist.x);
    float noiseYIntp = lerp(noiseXIntp0,noiseXIntp1,gridDist.y);
    return (noiseYIntp+1.0)/2.0;
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
    fragmentColor = vec4(vec3(fractalNoiseFunc(vPos)),1.0);
}