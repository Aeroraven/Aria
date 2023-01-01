#version 300 es

precision highp float;

in highp vec2 vTex;
in highp vec3 vPos;
uniform sampler2D uDiffuse;

out vec4 fragmentColor;

const int splitParts = 15;

float randomScalar(vec3 worldPos){
    float d = dot(vec3(114.1919, 514.810, 37.719),sin(worldPos));
    return fract(sin(d)*114514.1919810);
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
    
    float noise00 = randomScalar(gridMinPos+vec3(0.0,0.0,0.0));
    float noise10 = randomScalar(gridMinPos+vec3(gridDeltaPos.x,0.0,0.0));
    float noise01 = randomScalar(gridMinPos+vec3(0.0,gridDeltaPos.y,0.0));
    float noise11 = randomScalar(gridMinPos+vec3(gridDeltaPos.x,gridDeltaPos.y,0.0));

    float noiseXIntp0 = lerp(noise00,noise10,gridDist.x);
    float noiseXIntp1 = lerp(noise01,noise11,gridDist.x);
    float noiseYIntp = lerp(noiseXIntp0,noiseXIntp1,gridDist.y);
    return noiseYIntp;
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