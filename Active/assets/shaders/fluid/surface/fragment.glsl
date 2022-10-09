#version 300 es
precision highp float;
in mediump vec4 vColor;
in mediump vec2 vTex;
in mediump vec3 vNorm;
in mediump vec4 vFragPos;
in mediump vec4 vModelPos;

uniform sampler2D uDiffuse; //Color
uniform sampler2D uSpecular; //Pos
uniform vec3 uCamPos;
uniform mat4 uModel;
uniform mat4 uProj;
uniform mat4 uShadowOrtho;
uniform mat4 uLightModel[20];

out vec4 fragmentColor;

void main(){
    vec3 lightDir = (vec4(0.0,1.0,0.0,0.0)).xyz;
    float diffuse = max(0.0,dot(normalize(vNorm),lightDir));
    vec4 difColor = vec4(vec3(0.0,0.5,1.0)*(diffuse*0.5+0.5),1.0);
    //Surface Refraction Starts Here
    int found = 0;
    int invalidCounter = 0;
    const float ETA_FLUID = 1.30;
    const float MARCH_DELTA_LEN = 0.005;
    const int MARCH_STEPS = 800;
    const int MAX_TRIES = 500;
    const float EPS = 1e-2;
    vec3 eyeRay = normalize(vec3(0.0,0.0,-1.0)); //OR: vModelPos.xyz;
    vec3 startPoint = vModelPos.xyz;
    vec3 surfNorm = normalize(vNorm);
    vec3 refractRay = refract(eyeRay,surfNorm,ETA_FLUID);
    vec3 marchDelta = refractRay * MARCH_DELTA_LEN;
    vec4 envPos = vec4(0.0);
    vec3 curPos = startPoint;

    for(int i=0;i<MARCH_STEPS;i++){
        curPos=startPoint + marchDelta * float(i);
        //Find pos map
        vec4 curPosProjH = uProj * vec4(curPos,1.0);
        vec3 curPosProj = curPosProjH.xyz / curPosProjH.w;
        curPosProj = (curPosProj+1.0)*0.5;
        envPos = texture(uSpecular,curPosProj.xy); //model pos
        if(envPos.z-0.1>curPos.z){
            invalidCounter++;
            if(invalidCounter > 10){
                break;
            }
        }
        if(abs(envPos.z-curPos.z)<=EPS){
            vec4 envColor = texture(uDiffuse,curPosProj.xy);
            fragmentColor = envColor*0.5 + difColor*0.5;
            found = 1;
            break;
        }
    }
    //Binary search 
    if(found==0){
        vec3 lp = curPos;
        vec3 rp = startPoint;
        for(int i=0;i<MAX_TRIES;i++){
            vec3 md = (lp+rp)/2.0;
            vec4 vpCoordH = uProj * vec4(md,1.0);
            vec3 vpCoord = vpCoordH.xyz / vpCoordH.w;
            vec2 vpTex = vpCoord.xy * 0.5 + 0.5;
            vec4 vpDest = texture(uSpecular,vpTex);
            if(abs(vpDest.z - md.z)<EPS){
                found=1;
                vec4 envColor = vec4(texture(uDiffuse,vpTex).xyz,1.5);  
                fragmentColor = envColor*0.5 + difColor*0.5;
                break;
            }
            if(vpDest.z>md.z){
                rp = md;
            }else{
                lp = md;
            }
        }
    }
    if(found==0){
        fragmentColor = vec4(0.3)*0.5 + difColor*0.5;
    }
}