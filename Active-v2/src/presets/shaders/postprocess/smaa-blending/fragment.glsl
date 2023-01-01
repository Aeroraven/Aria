#version 300 es
precision highp float;
uniform sampler2D uSourceFrame;
out vec4 fragmentColor;
in vec2 vTex;

const float revectorFactor = 0.5;
const float thres = 0.35;
const float eps = 1e-9;
const int maxSteps = 64;

float intpl(float leftPos, float leftVal, float rightPos, float rightVal, float curPos){
    float curDist = curPos - leftPos;
    return leftVal + (rightVal - leftVal) * (curDist)/(rightPos-leftPos);
}

float findArea(int leftType, int rightType, int leftDev, int rightDev, float leftOffset, float rightOffset, float curPos, float crit){
    float ret = 0.0;
    float area = 0.0;
    float midPos = (leftOffset + rightOffset)/2.0;
    if(leftType == 0){ //Left Flat
        if(rightType == 0 || rightType == 3){
            ret = 0.0;
            area = 0.0;
        }else if(rightType==1){ //Right UP
            if(leftDev<=rightDev){
                ret = 0.0;
            }else{
                ret = intpl(midPos,0.0,rightOffset,0.5,curPos);
                if(abs(curPos+crit*0.5-midPos)<crit){
                    ret = ret *  (curPos+crit*0.5-midPos)/crit * 0.5;
                }
            }
        }else{
            if(leftDev<=rightDev){
                ret = 0.0;
            }else{
                ret = intpl(midPos,0.0,rightOffset,-0.5,curPos);
                if(abs(curPos+crit*0.5-midPos)<crit){
                    ret = ret *  (curPos+crit*0.5-midPos)/crit * 0.5;
                }
            }
        }
    }else if(leftType==1){ //Left UP
        if(rightType==0){ //Right Flat
            if(leftDev<rightDev){
                ret = intpl(leftOffset,0.5,midPos,0.0,curPos);
                if(abs(curPos+crit*0.5-midPos)<crit){
                    ret = ret *  (-curPos-crit*0.5+midPos)/crit * 0.5;
                }
            }else{
                ret = 0.0;
            }
        }else if(rightType==1){//Right UP
            //REQ REFINE
            if(leftDev<rightDev){
                ret = intpl(leftOffset,0.5,midPos,0.0,curPos);
            }else{
                ret = intpl(midPos,0.0,rightOffset,0.5,curPos);
            }
            //ret = 1.0;
        } else{
            ret = intpl(leftOffset,0.5, rightOffset,-0.5,curPos);
        }
    }else if(leftType==2){ //Left Down
        if(rightType==0){ //Right Flat
            if(leftDev<rightDev){
                ret = intpl(leftOffset,-0.5,midPos,0.0,curPos);
                if(abs(curPos+crit*0.5-midPos)<crit){
                    ret = ret *  (-curPos-crit*0.5+midPos)/crit * 0.5;
                }
            }else{
                ret = 0.0;
            }
        }else if(rightType==2){ //Right Down
            //REQ REFINE
            if(leftDev<rightDev){
                ret = intpl(leftOffset,-0.5,midPos,0.0,curPos);
            }else{
                ret = intpl(midPos,0.0,rightOffset,-0.5,curPos);
            }
            if(ret>0.0){
                ret = 1.0; //PROBLEM HERE
            }
            
        }else{
            ret = intpl(leftOffset,-0.5, rightOffset,+0.5,curPos);
        }
    }else{ //Left Dual
        if(rightType==0 || rightType==3){
            ret = 0.0;
        }else if(rightType==1){ //Right Up
            ret = intpl(leftOffset,-0.5,rightOffset,0.5,curPos);
        }else if(rightType==2){ //Right Up
            ret = intpl(leftOffset,0.5,rightOffset,-0.5,curPos);
        }
    }
    return ret;
}

float blendTop(int dir){
    ivec2 texSize = textureSize(uSourceFrame, 0);
    vec2 texOffset = 1.0 / vec2(float(texSize.x),float(texSize.y));

    float leftOffset = vTex.x;
    float rightOffset = vTex.x;
    int leftDev = 0, rightDev = 0, leftType = 0, rightType = 0;

    
    //Find Left Edge: Check Channel Green
    for(int i=0;i<maxSteps;i++){
        float leftPos = -float(i) * texOffset.x + vTex.x;
        vec4 cl = texture(uSourceFrame,vec2(leftPos,vTex.y));
        float ck = 0.0;
        if(dir==0){
            ck = cl.g;
        }else{
            ck = cl.a;
        }
        if(ck > eps){
            leftOffset = leftPos;
            leftDev = i;
            continue;
        }
        break;
    }

    //Find Right Edge: Check Channel Green
    for(int i=0;i<maxSteps;i++){
        float rightPos = float(i) * texOffset.x + vTex.x;
        vec4 cl = texture(uSourceFrame,vec2(rightPos,vTex.y));
        float ck = 0.0;
        if(dir==0){
            ck = cl.g;
        }else{
            ck = cl.a;
        }
        if(ck > eps){
            rightOffset = rightPos;
            rightDev = i;
            continue;
        }
        break;
    }

    //For Left Edge, determine vertical type, 1 means edge goes up
    if(dir==0){
        if(texture(uSourceFrame, vec2(leftOffset, vTex.y -  texOffset.y)).r > eps){
            leftType += 1;
        }
        if(texture(uSourceFrame, vec2(leftOffset, vTex.y)).r > eps){
            leftType += 2;
        }

        //The same for the right
        if(texture(uSourceFrame, vec2(rightOffset +  texOffset.x, vTex.y - texOffset.y)).r > eps){
            rightType += 1;
        }
        if(texture(uSourceFrame, vec2(rightOffset +  texOffset.x, vTex.y )).r > eps){
            rightType += 2;
        }
    }else{
        if(texture(uSourceFrame, vec2(leftOffset, vTex.y )).r > eps){
            leftType += 1;
        }
        if(texture(uSourceFrame, vec2(leftOffset, vTex.y + texOffset.y)).r > eps){
            leftType += 2;
        }

        //The same for the right
        if(texture(uSourceFrame, vec2(rightOffset +  texOffset.x, vTex.y)).r > eps){
            rightType += 1;
        }
        if(texture(uSourceFrame, vec2(rightOffset +  texOffset.x, vTex.y + texOffset.y)).r > eps){
            rightType += 2;
        }
    }
    //SMAA: Sharp Geometric Feature Refinement
    vec4 smaaEConds = vec4(0.0);
    if(dir==0){
        //Top Edge
        float e1v = texture(uSourceFrame,vec2(leftOffset, vTex.y-2.0*texOffset.y)).r;
        float e2v = texture(uSourceFrame,vec2(rightOffset +  texOffset.x, vTex.y-2.0*texOffset.y)).r;
        float e3v = texture(uSourceFrame,vec2(leftOffset, vTex.y+1.0*texOffset.y)).r;
        float e4v = texture(uSourceFrame,vec2(rightOffset +  texOffset.x, vTex.y+1.0*texOffset.y)).r;
        smaaEConds = vec4(e1v,e2v,e3v,e4v);
    }else{
        //Bottom Edge
        float e1v = texture(uSourceFrame,vec2(leftOffset, vTex.y-1.0*texOffset.y)).r;
        float e2v = texture(uSourceFrame,vec2(rightOffset +  texOffset.x, vTex.y-1.0*texOffset.y)).r;
        float e3v = texture(uSourceFrame,vec2(leftOffset, vTex.y+2.0*texOffset.y)).r;
        float e4v = texture(uSourceFrame,vec2(rightOffset +  texOffset.x, vTex.y+2.0*texOffset.y)).r;
        smaaEConds = vec4(e1v,e2v,e3v,e4v);
    }

    //Conditions
    leftOffset -= 0.5*texOffset.x;
    rightOffset += 0.5*texOffset.x;
    float ret = findArea(leftType, rightType, leftDev, rightDev, leftOffset, rightOffset, vTex.x, texOffset.x);
    
    //SMAA: Sharp Geometric Feature Refinement II
    if(ret>0.0){
        if(abs(leftOffset)<abs(rightOffset) && smaaEConds.x > 1e-3){
            ret = ret * revectorFactor;
        }else if(abs(leftOffset)>=abs(rightOffset) && smaaEConds.y > 1e-3){
            ret = ret * revectorFactor;
        }
    }else{
        if(abs(leftOffset)<abs(rightOffset) && smaaEConds.z > 1e-3){
            ret = ret * revectorFactor;
        }else if(abs(leftOffset)>=abs(rightOffset) && smaaEConds.w > 1e-3){
            ret = ret * revectorFactor;
        }
    }
    return ret;
}

float blendLeft(int dir){
    ivec2 texSize = textureSize(uSourceFrame, 0);
    vec2 texOffset = 1.0 / vec2(float(texSize.x),float(texSize.y));

    float leftOffset = vTex.y;
    float rightOffset = vTex.y;
    int leftDev = 0, rightDev = 0, leftType = 0, rightType = 0;

    //Find Bottom Edge: Check Channel Green
    for(int i=0;i<maxSteps;i++){
        float leftPos = -float(i) * texOffset.y + vTex.y;
        vec4 cl = texture(uSourceFrame,vec2(vTex.x,leftPos));
        float ck = 0.0;
        if(dir==0){
            ck = cl.r;
        }else{
            ck = cl.b;
        }
        if(ck > eps){
            leftOffset = leftPos;
            leftDev = i;
            continue;
        }
        break;
    }

    //Find Bottom Edge: Check Channel Green
    for(int i=0;i<maxSteps;i++){
        float rightPos = float(i) * texOffset.y + vTex.y;
        vec4 cl = texture(uSourceFrame,vec2(vTex.x,rightPos));
        float ck = 0.0;
        if(dir==0){
            ck = cl.r;
        }else{
            ck = cl.b;
        }
        if(ck > eps){
            rightOffset = rightPos;
            rightDev = i;
            continue;
        }
        break;
    }
    if(dir==0){
        //For Top Edge, determine vertical type, 1 means edge goes left
        if(texture(uSourceFrame, vec2(vTex.x -  texOffset.x, leftOffset)).g > eps){
            leftType += 1;
        }
        if(texture(uSourceFrame, vec2(vTex.x, leftOffset)).g > eps){
            leftType += 2;
        }

        //The same for the right
        if(texture(uSourceFrame, vec2(vTex.x -  texOffset.x, rightOffset)).a > eps){
            rightType += 1;
        }
        if(texture(uSourceFrame, vec2(vTex.x, rightOffset)).a > eps){
            rightType += 2;
        }
    }else{
        //For Top Edge, determine vertical type, 1 means edge goes left
        if(texture(uSourceFrame, vec2(vTex.x , leftOffset)).g > eps){
            leftType += 1;
        }
        if(texture(uSourceFrame, vec2(vTex.x +  texOffset.x, leftOffset)).g > eps){
            leftType += 2;
        }

        //The same for the right
        if(texture(uSourceFrame, vec2(vTex.x, rightOffset)).a > eps){
            rightType += 1;
        }
        if(texture(uSourceFrame, vec2(vTex.x +  texOffset.x, rightOffset)).a > eps){
            rightType += 2;
        }
    }
    //SMAA: Sharp Geometry Feature Refinement
    vec4 smaaEConds = vec4(0.0);
    if(dir==0){
        //Left Edge
        float e1v = texture(uSourceFrame,vec2(vTex.x-2.0*texOffset.x,leftOffset)).t;
        float e2v = texture(uSourceFrame,vec2(vTex.x-2.0*texOffset.x,rightOffset + texOffset.y)).t;
        float e3v = texture(uSourceFrame,vec2(vTex.x+1.0*texOffset.x,leftOffset)).t;
        float e4v = texture(uSourceFrame,vec2(vTex.x+1.0*texOffset.x,rightOffset + texOffset.y)).t;
        smaaEConds = vec4(e1v,e2v,e3v,e4v);
    }else{
        float e1v = texture(uSourceFrame,vec2(vTex.x-1.0*texOffset.x,leftOffset)).t;
        float e2v = texture(uSourceFrame,vec2(vTex.x-1.0*texOffset.x,rightOffset + texOffset.y)).t;
        float e3v = texture(uSourceFrame,vec2(vTex.x+2.0*texOffset.x,leftOffset)).t;
        float e4v = texture(uSourceFrame,vec2(vTex.x+2.0*texOffset.x,rightOffset + texOffset.y)).t;
        smaaEConds = vec4(e1v,e2v,e3v,e4v);
    }
    
    //Conditions
    leftOffset -= 0.5*texOffset.y;
    rightOffset += 0.5*texOffset.y;
    float ret = findArea(leftType, rightType, leftDev, rightDev, leftOffset, rightOffset, vTex.y, texOffset.y);
    
    //SMAA: Sharp Geometry Feature Refinement
    if(ret>0.0){
        if(abs(leftOffset)<abs(rightOffset) && smaaEConds.x > 1e-3){
            ret = ret * revectorFactor;
        }else if(abs(leftOffset)>=abs(rightOffset) && smaaEConds.y > 1e-3){
            ret = ret * revectorFactor;
        }
    }else{
        if(abs(leftOffset)<abs(rightOffset) && smaaEConds.z > 1e-3){
            ret = ret * revectorFactor;
        }else if(abs(leftOffset)>=abs(rightOffset) && smaaEConds.w > 1e-3){
            ret = ret * revectorFactor;
        }
    }
    return ret;
}


void main(){
    if(dot(vec4(1.0), texture(uSourceFrame, vTex))< eps){
        discard;
    }

    float retTop = blendTop(0);
    float retBottom = blendTop(1);
    float retLeft = blendLeft(0);
    float retRight = blendLeft(1);

    fragmentColor = clamp(vec4(retLeft,retTop,-retRight,-retBottom),vec4(0.0), vec4(1.0));  
}