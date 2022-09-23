#version 300 es

precision highp float;

in highp vec2 vTex;
uniform sampler2D uDiffuse;

out vec4 fragmentColor;

float luminance(vec4 x){
    return dot(x.rgb,vec3(0.213,0.715,0.072));
}

void main(){
    //Neighbour
    ivec2 texSize = textureSize(uDiffuse, 0);
    vec2 texOffset = 1.0 / vec2(float(texSize.x),float(texSize.y));
    vec2 LT = vTex+vec2(-1.0,-1.0)*texOffset;
    vec2 L = vTex+vec2(-1.0,0.0)*texOffset;
    vec2 LB = vTex+vec2(-1.0,1.0)*texOffset;
    vec2 RT = vTex+vec2(1.0,-1.0)*texOffset;
    vec2 R = vTex+vec2(1.0,0.0)*texOffset;
    vec2 RB = vTex+vec2(1.0,1.0)*texOffset;
    vec2 T = vTex+vec2(0.0,-1.0)*texOffset;
    vec2 B = vTex+vec2(0.0,1.0)*texOffset;

    vec4 cLT = texture(uDiffuse,LT);
    vec4 cL = texture(uDiffuse,L);
    vec4 cLB = texture(uDiffuse,LB);
    vec4 cRT = texture(uDiffuse,RT);
    vec4 cR = texture(uDiffuse,R);
    vec4 cRB = texture(uDiffuse,RB);
    vec4 cT = texture(uDiffuse,T);
    vec4 cB = texture(uDiffuse,B);
    vec4 cM = texture(uDiffuse,vTex);

    float lLT = luminance(cLT);
    float lL = luminance(cL);
    float lLB = luminance(cLB);
    float lRT = luminance(cRT);
    float lR = luminance(cR);
    float lRB = luminance(cRB);
    float lT = luminance(cT);
    float lB = luminance(cB);
    float lM = luminance(cM);
    

    float maxLumina = max(lM,max(lB,max(lT,max(lL,lR))));
    float minLumina = min(lM,min(lB,min(lT,min(lL,lR))));

    const float THRESH_1 = 0.063;
    const float THRESH_2 = 0.0312;
    if(abs(maxLumina-minLumina)>max(THRESH_2,THRESH_1*maxLumina)){
        //FXAA Here
        float flt = 2.0*(lL+lB+lT+lR) + (lLT+lLB+lRT+lRB)*1.0;
        flt/=12.0;
        flt-=lM;
        flt=abs(flt);
        float coef = clamp(flt/(maxLumina-minLumina),0.0,1.0);
        coef = smoothstep(0.0,1.0,coef);
        coef *= coef;

        float vertical = abs(lT+lB-2.0*lM)*2.0+abs(lLT+lLB-2.0*lL)+abs(lRT+lRB-2.0*lR);
        float horizontal = abs(lR+lL-2.0*lM)*2.0+abs(lLT+lRT-2.0*lT)+abs(lLB+lRB-2.0*lB);
        bool dir = vertical > horizontal;
        float positive = 0.0;
        float negative = 0.0;
        vec2 texStep = vec2(0.0);
        vec2 scanDir = vec2(0.0);
        vec3 dbgColor = vec3(1.0);
        float gradv = 0.0;
        if(dir){
            //Vertical
            positive = lB;
            negative = lT;
            texStep.y = texOffset.y;
            scanDir.x = texOffset.x;
            dbgColor = vec3(1.0,0.0,0.0);
            gradv = positive;
        }else{
            //Horizontal
            negative = lL;
            positive = lR;
            texStep.x = texOffset.x;
            scanDir.y = texOffset.y;
            dbgColor = vec3(0.0,1.0,0.0);
            gradv = positive;
        }

        if(abs(positive-lM)<abs(negative-lM)){
            texStep = -texStep;
            dbgColor.b = 1.0;
            gradv = negative;
        }

        //Mixing Coef
        vec2 startingPos = vTex+texStep*0.5;
        const int maxStep = 10;
        vec4 ccolor = (texture(uDiffuse,vTex)+texture(uDiffuse,vTex+texStep))*0.5;
        float lccolor = luminance(ccolor);
        float ldir = 0.0;
        float rdir = 0.0;
        vec4 lcolor = vec4(0.0);
        vec4 rcolor = vec4(0.0);
        float lstep = 0.0;
        float rstep = 0.0;

        //Search Neg dir
        for(int i=0;i<maxStep;i++){
            lstep += 1.0;
            vec2 lpix = startingPos - lstep * scanDir;
            if(lpix.x<0.0||lpix.x>1.0||lpix.y<0.0||lpix.y>1.0){
                break;
            }
            float lluma = luminance(texture(uDiffuse,lpix)); 
            lcolor = texture(uDiffuse,lpix);
            ldir = sign(lluma-lccolor);
            if(abs(lluma-lccolor)>gradv*0.25){
                break;
            }
        }

        //Search Pos Dir
        for(int i=0;i<maxStep;i++){
            rstep += 1.0;
            vec2 lpix = startingPos + rstep * scanDir;
            if(lpix.x<0.0||lpix.x>1.0||lpix.y<0.0||lpix.y>1.0){
                break;
            }
            float lluma = luminance(texture(uDiffuse,lpix)); 
            rcolor = texture(uDiffuse,lpix);
            rdir = sign(lluma-lccolor);
            if(abs(lluma-lccolor)>gradv*0.25){
                break;
            }
        }

        //Get
        float ecoef = 0.0; 
        if(lstep<rstep){
            ecoef = 0.5 - lstep/(lstep+rstep);
            if(ldir==sign(lM-lccolor)){
                ecoef = 0.0;
            }
        }else{
            ecoef = 0.5 - rstep/(lstep+rstep);
            if(rdir==sign(lM-lccolor)){
                ecoef = 0.0;
            }
        }
        coef = max(ecoef,coef);
        vec4 thisColor = texture(uDiffuse,vTex);
        vec4 destColor = texture(uDiffuse,vTex+texStep);
        fragmentColor = mix(thisColor,destColor,coef) ; //   vec4(vec3(coef),1.0)
        //fragmentColor = vec4(dbgColor,1.0);   
        
    } else{
        fragmentColor = texture(uDiffuse,vTex);  
        //fragmentColor = vec4(0.0);   
    }

    
}