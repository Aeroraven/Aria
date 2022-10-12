#version 300 es
precision highp float;
in vec4 aVert;
in vec2 aTex;
in vec3 aNorm;

out highp vec4 vColor;
out highp vec2 vTex;
out highp vec3 vNorm;
out highp vec4 vFragPos;
out highp vec4 vModelPos;

out highp vec3 vOldPos;
out highp vec3 vNewPos;


uniform mat4 uModelInvTrans;
uniform mat4 uModel;
uniform mat4 uProj;

const float uPlainSurfaceHeight = -4.0;

uniform sampler2D uDiffuse; //Color
uniform sampler2D uSpecular; //Pos

vec3 projIntersect(vec3 startPoint,vec3 refractRay){
    //Assumes light casts vertically
    vec3 retPos = startPoint + 2.0*refractRay;
    //Ray marching params

    int invalidCounter = 0;
    int found = 0;
    vec3 st = startPoint;
    const int MARCH_STEPS = 600;
    const int ITERS = 16;
    const float EPS = 3e-3;
    float marchDeltaLen = 0.002;

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
                if(invalidCounter > 50){
                    break;
                }
            }
            if(length(envPos.xyz-curPos)<=EPS){
                retPos = envPos.xyz;
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
    return retPos;
}

void main(){

    //Basic
    vModelPos = uModel * aVert;
    gl_Position = uProj * vModelPos;
    vNorm = (uModel * vec4(normalize(aNorm),0.0)).xyz;
    vTex = aTex;
    vFragPos = aVert;

    //Proc
    const float ETA_WATER = 1.3;
    vec3 startPoint = vModelPos.xyz;
    vec3 startPointIdeal = (uModel * vec4(aVert.x,uPlainSurfaceHeight,aVert.z,1.0)).xyz;
    vec3 viewDir = (uModel * vec4(0.0,-1.0,0.0,0.0)).xyz;
    vec3 refractRay = refract(viewDir,vNorm,ETA_WATER);
    
    //Projected position under no distrubance
    vOldPos = projIntersect(startPointIdeal,viewDir);

    //Projected position under waved surface
    vNewPos = projIntersect(startPoint,refractRay);

    gl_Position = uProj * vec4(vNewPos.xz,vNewPos.y,1.0).xzyw;
}