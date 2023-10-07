#version 300 es
precision highp float;
in highp vec4 vColor;
in highp vec2 vTex;
in highp vec3 vNorm;
in highp vec4 vFragPos;
in highp vec4 vModelPos;

uniform sampler2D uDiffuse; //Color
uniform sampler2D uSpecular; //Pos
uniform sampler2D uNormal; //Caustics
uniform vec3 uCamPos;
uniform mat4 uModel;
uniform mat4 uProj;
uniform mat4 uShadowOrtho;
uniform mat4 uLightModel[20];

out vec4 fragmentColor;

void main(){
    vec3 lightDir = (vec4(0.0,1.0,0.0,0.0)).xyz;
    float diffuse = max(0.0,dot(normalize(vNorm),lightDir));
    vec4 difColor = vec4(vec3(0.1,0.5,1.0)*(diffuse*0.5+0.5),1.0);
    //Surface Refraction Starts Here
    int found = 0;
    int invalidCounter = 0;
    const float ETA_FLUID = 1.30;
    
    const int MARCH_STEPS = 500;
    const int MAX_TRIES = 50;
    const int ITERS = 4;
    const float EPS = 4e-3;
    const float EPS2 = 1e-3;

    float marchDeltaLen = 0.006;
    vec3 eyeRay = normalize(vec3(0.0,0.0,-1.0)); //OR: vModelPos.xyz;
    vec3 startPoint = vModelPos.xyz;
    vec3 surfNorm = normalize(vNorm);
    vec3 refractRay = normalize(refract(eyeRay,surfNorm,ETA_FLUID));
    vec3 marchDelta = refractRay * marchDeltaLen;
    vec4 envPos = vec4(0.0);
    vec3 curPos = startPoint;
    vec3 lastPos = startPoint;
    for(int T=0;T<ITERS;T++){
        invalidCounter = 0;
        for(int i=1;i<MARCH_STEPS;i++){
            curPos=startPoint + marchDelta * float(i);
            //Find pos map
            vec4 curPosProjH = uProj * vec4(curPos,1.0);
            vec3 curPosProj = curPosProjH.xyz / curPosProjH.w;
            curPosProj = (curPosProj+1.0)*0.5;
            envPos = texture(uSpecular,curPosProj.xy); //model pos
            if(curPosProj.x<0.0||curPosProj.x>1.0||curPosProj.y<0.0||curPosProj.y>1.0){
                break;
            }
            if(envPos.z>curPos.z){
                invalidCounter++;
                if(invalidCounter > 100){
                    break;
                }
            }
            if(length(envPos.xyz-curPos)<=EPS){
                vec4 envColor = texture(uDiffuse,curPosProj.xy);
                vec4 cauColor = vec4(texture(uNormal,curPosProj.xy).xyz,1.0);  
                fragmentColor = envColor*0.5 + difColor*0.5 + cauColor * 0.3;
                found = 1;
                break;
            }
            if(invalidCounter==0){
                lastPos = curPos;
            }
        }
        if(found==1){
            break;
        }
        startPoint = lastPos;
        marchDeltaLen *= 0.25;
        marchDelta = refractRay * marchDeltaLen;
    }
    
    //Binary search 
    if(found==0){
        vec3 lp = curPos; //z^
        vec3 rp = lastPos; //zv
        for(int i=0;i<MAX_TRIES;i++){
            vec3 md = (lp+rp)/2.0;
            vec4 vpCoordH = uProj * vec4(md,1.0);
            vec3 vpCoord = vpCoordH.xyz / vpCoordH.w;
            vec2 vpTex = (vpCoord.xy+1.0)*0.5;
            vec4 vpDest = texture(uSpecular,vpTex);
            if(length(vpDest.xyz - md)<=EPS2){
                found=1;
                vec4 envColor = vec4(texture(uDiffuse,vpTex).xyz,1.0);  
                vec4 cauColor = vec4(texture(uNormal,vpTex).xyz,1.0);  
                fragmentColor = envColor*0.5 + difColor*0.5 + cauColor * 0.3;
                break;
            }
            if(vpDest.z<=md.z){
                rp = md;
            }else{
                lp = md;
            }
        }
    }
    if(found==0){
        fragmentColor = vec4(0.3)*0.5 + difColor*0.5;
    }
    //fragmentColor = vec4(vec3(refractRay.y),1.0);
}