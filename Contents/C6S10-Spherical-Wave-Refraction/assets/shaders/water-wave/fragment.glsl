#version 300 es

precision mediump float;

in mediump vec2 vTex;

uniform sampler2D uDiffuse;
uniform float uCamAspect;
uniform float uTimestamp;

out vec4 fragmentColor;

const float GAMMA = 2.2;

struct WaveResponse{
    float response;
    vec3 normal;
};

WaveResponse sphericalWave(vec2 source,vec2 pos,float freq,float phase,float ampl,float velocity){
    float dist = length(pos-source);
    float curPhase = freq*(dist-velocity*uTimestamp)+phase;
    float response = 1.0/dist*sin(curPhase)*ampl;
    
    //Derivatives
    float drc = 1.0/pow(dist,3.0); //Common part of dr/dx & dr/dy;
    float drx = -(pos.x-source.x)*drc;
    float dry = -(pos.y-source.y)*drc;

    float dfx1 = 1.0/pow(dist,2.0)*drx*sin(curPhase);
    float dfx2 = -1.0/dist*cos(curPhase)*freq*drx;
    float dfx = dfx1+dfx2;

    float dfy1 = 1.0/pow(dist,2.0)*dry*sin(curPhase);
    float dfy2 = -1.0/dist*cos(curPhase)*freq*dry;
    float dfy = dfy1+dfy2;

    dfx*=ampl;
    dfy*=ampl;
    
    float dfz = 1.0;
    vec3 waveNormal = normalize(vec3(dfx,dfy,dfz));

    WaveResponse ret = WaveResponse(response,waveNormal);
    return ret;
}

WaveResponse waveFunction(vec2 pos){
    WaveResponse ret = sphericalWave(vec2(-2.0,2.0)*vec2(uCamAspect,1.0),pos,12.0,0.0,0.008,0.0004);
    return ret;
}

vec3 gammaEncoding(vec3 col){
    return pow(col,vec3(GAMMA));
}

vec3 gammaRectification(vec3 col){
    return pow(col,vec3(1.0/GAMMA));
}

void main(){
    const float ETA_WATER = 1.33;
    //This assumes all are at the same depth
    //Ray-marching can be used for optimization
    const float DISP_Z = -5.0; 
    vec2 vTexN = 2.0 * (vTex - 0.5);

    //Wave Response
    WaveResponse wave = waveFunction(vTexN*vec2(uCamAspect,1.0));
    float resp = wave.response * 15.0;
    resp = resp * 0.5 + 0.5;
    resp = smoothstep(0.0,1.0,resp);
    vec3 waveColor = gammaEncoding(vec3(0.5294,0.8078,0.9216)) * resp;

    //Refract
    vec3 refractPoint = vec3(vTexN,wave.response);
    vec3 inputRay = vec3(0.0,0.0,-1.0);
    vec3 outputRay = refract(inputRay,wave.normal,ETA_WATER);
    float marchDist = refractPoint.z - DISP_Z;
    vec3 mappingPoint = refractPoint + marchDist * outputRay;

    //Texture Mixing
    vec2 mappedTex = mappingPoint.xy / 2.0 + 0.5;
    vec3 texColor = gammaEncoding(texture(uDiffuse,mappedTex).rgb);
    fragmentColor = vec4(gammaRectification(texColor*0.3+waveColor*0.7),1.0);   
}