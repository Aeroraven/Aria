#version 300 es

precision mediump float;

in highp vec2 vTex;
uniform sampler2D uDiffuse; //WorldPos (X,Y,Z)
uniform sampler2D uSpecular; //World Normal 
uniform sampler2D uNormal; //World Scene 

out vec4 fragmentColor;

uniform mat4 uProj;

void main(){
    const int MAX_TRIES = 900;
    const int MAX_STEPS = 600;
    const float STEP_DELTA = 0.2;
    //Debug
    fragmentColor = vec4(vec3(0.0),1.0);   
    
    vec3 destPos = texture(uDiffuse,vTex).xyz;
    float reflAttr = texture(uDiffuse,vTex).a;
    vec3 viewDir = normalize(destPos);
    vec3 normDirx = texture(uSpecular,vTex).xyz/ texture(uSpecular,vTex).a;
    vec3 normDir  = normalize(normDirx*2.0-1.0);
    vec3 reflDir = normalize(reflect(viewDir,normDir));
    if(reflAttr<0.5){
        vec3 curPos = destPos;
        int found = 0;
        vec3 vpCoord = vec3(0.0);
        for(int i=0;i<MAX_STEPS;i++){
            curPos = STEP_DELTA * float(i+1) * reflDir + destPos;
            vec4 vpCoordH = uProj * vec4(curPos,1.0);
            vpCoord = vpCoordH.xyz / vpCoordH.w;
            vec2 vpTex = vpCoord.xy * 0.5 + 0.5;
            vec4 vpDest = texture(uDiffuse,vpTex);
            if(vpDest.z>curPos.z){
                if(abs(vpDest.z - curPos.z)<5e-3 && vpDest.a>0.5){
                    found=1;
                    fragmentColor = vec4(texture(uNormal,vpTex).xyz,0.5);   
                }
                break;
            }
        }
        if(found==0){
            vec3 lp = destPos;
            vec3 rp = curPos;
            for(int i=0;i<MAX_TRIES;i++){
                vec3 md = (lp+rp)/2.0;
                vec4 vpCoordH = uProj * vec4(md,1.0);
                vpCoord = vpCoordH.xyz / vpCoordH.w;
                vec2 vpTex = vpCoord.xy * 0.5 + 0.5;
                vec4 vpDest = texture(uDiffuse,vpTex);
                if(abs(vpDest.z - md.z)<1e-3 && vpDest.a>0.5){
                    found=1;
                    fragmentColor = vec4(texture(uNormal,vpTex).xyz,0.5);  
                    break;
                }
                if(vpDest.z>md.z){
                    rp = md;
                }else{
                    lp = md;
                }
            }
        }
    }else{
        fragmentColor = vec4(vec3(0.0),1.0);   
    }
    
}