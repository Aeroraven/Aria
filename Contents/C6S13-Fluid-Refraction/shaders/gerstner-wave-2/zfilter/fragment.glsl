#version 300 es

precision mediump float;

in mediump vec2 vTex;
uniform sampler2D uDiffuse;
uniform sampler2D uSpecular;
uniform sampler2D uNormal;
uniform sampler2D uAlbedo;
uniform mat4 uModel;
uniform mat4 uProj;

uniform vec3 uCamPos;

out vec4 fragmentColor;

void main(){
    const int MAX_TRIES = 50;
    const int MAX_STEPS = 50;
    const float STEP_DELTA = 8.0;

    vec4 waterPos = texture(uSpecular,vTex);   
    vec4 scenePos = texture(uNormal,vTex);  
    vec4 swpos = uProj * uModel * vec4(waterPos.xyz,1.0);
    vec4 scpos = uProj * uModel * vec4(scenePos.xyz,1.0);
    swpos/=swpos.w;
    scpos/=scpos.w;
    vec4 frag = texture(uDiffuse,vTex);
    if(swpos.z>=scpos.z || waterPos.w < 0.3 || vTex.x<0.0 || vTex.x>1.0){
        frag = texture(uDiffuse,vTex);   
    }else{
        vec3 L = vec3(0.0,1.0,0.0);
        vec3 N = texture(uAlbedo,vTex).xyz;  
        //Ray Marching Here ?
        vec3 startPos = (vec4(waterPos.xyz,1.0)).xyz;
        vec3 viewDir = normalize(startPos-uCamPos);
        vec3 refraDir = refract(viewDir,N,1.33);

        vec3 curPos = startPos;
        int found = 0;
        vec3 vpCoord = vec3(0.0);
        vec4 fragCl = vec4(0.0);
        for(int i=0;i<MAX_STEPS;i++){
            curPos = STEP_DELTA * float(i+1) * refraDir + startPos;
            vec4 vpCoordH = uProj * vec4(curPos,1.0);
            vpCoord = vpCoordH.xyz / vpCoordH.w;
            vec2 vpTex = vpCoord.xy * 0.5 + 0.5;
            vec4 vpDest = texture(uNormal,vpTex);
            if(vpDest.z>curPos.z){
                if(abs(vpDest.z - curPos.z)<5e-3 && vpDest.a>0.5){
                    found=1;
                    fragCl = vec4(texture(uDiffuse,vpTex).xyz,0.5);   
                }
                break;
            }
        }
        if(found==0){
            vec3 lp = startPos;
            vec3 rp = curPos;
            for(int i=0;i<MAX_TRIES;i++){
                vec3 md = (lp+rp)/2.0;
                vec4 vpCoordH = uProj * vec4(md,1.0);
                vpCoord = vpCoordH.xyz / vpCoordH.w;
                vec2 vpTex = vpCoord.xy * 0.5 + 0.5;
                vec4 vpDest = texture(uDiffuse,vpTex);
                if(abs(vpDest.z - md.z)<1e-3 && vpDest.a>0.5){
                    found=1;
                    fragCl = vec4(texture(uDiffuse,vpTex).xyz,0.5);  
                    break;
                }
                if(vpDest.z>md.z){
                    rp = md;
                }else{
                    lp = md;
                }
            }
        }

        //Under Water
        frag = fragCl*0.4 + 0.6 * vec4(0.5294,0.8078,0.9216,1.0)*max(pow(dot(L,N),2.0),0.0);   

    }
    fragmentColor = frag;
    
}