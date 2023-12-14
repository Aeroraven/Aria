#version 300 es
precision highp float;

// 每一个随机旋转的样本(受到3个随机0-1均匀分布随机数控制)
// 也可以不传那么多aPos，例如：https://thebookofshaders.com/11/
in vec3 aPos;

//--------------------------------
// Uniform 变量 (这些变量在AriaStageInternalAngleReproj.ts中控制)
//--------------------------------

// 可视化所需要的几个矩阵
uniform mat4 uProj;
uniform mat4 uViewOrtho;
uniform mat4 uModelView;
uniform mat4 uLocalMat;

// 四个顶点的位置(L-左，R-右，T-上，B-下)
uniform vec3 uLT;
uniform vec3 uLB;
uniform vec3 uRT;
uniform vec3 uRB;

// 模拟相机的焦距
uniform float uFocalX;
uniform float uFocalY;

// 空间矩形的平移
uniform vec3 uTranslation;

// 旋转角度上下限
uniform float uRotLb;
uniform float uRotRb;

// 旋转轴
uniform vec3 uRotAxis;

// 点的大小
uniform float pSize;

// 扰动偏移
uniform float disturbX;
uniform float disturbY;

// 是否以法向量位置作为顶点着色器输出
uniform float outputNormal;

// 预设法向量（即结果之保持空间矩形的法向量为该值）
uniform vec3 preNormal;

// 是否使用均匀分布的随机旋转SO(3)
uniform float useUniformDist;

// 是否对顶点进行排序(即左上角为第一个顶点)
uniform float sortVertex;

// 是否强制法向量为逆时针
uniform float forceCounterClockwiseNormal;


//--------------------------------
// Varying 变量（这些变量值传递给片段着色器shader.frag）
//--------------------------------

// 进行旋转后的四个顶点在相机坐标系下的位置
out vec4 vLT;
out vec4 vLB;
out vec4 vRT;
out vec4 vRB;

// 进行旋转后的四个顶点在相机坐标系下的位置，加上扰动
out vec4 vLTd;
out vec4 vLBd;
out vec4 vRTd;
out vec4 vRBd;

// 顶点内角值
out vec4 vCol;

// 顶点内角值（写冗余了www）
out vec4 vPos;

const float PI2 = 6.283185307179586476925286766559;


//这是原始的旋转矩阵产生算法，该算法产生非均匀SO(3)矩阵
mat4 rotationMatrix(vec3 axis, float angle){
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;

    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

//这是均匀分布的SO(3)矩阵产生算法，详细算法参考：
// https://dl.acm.org/doi/book/10.5555/531430 III.4小节
// https://en.wikipedia.org/wiki/Householder_transformation
mat3 uniformRotationMatrix(float x1,float x2,float x3){
    x1 = x1*PI2;
    x2 = x2*PI2;
    mat3 R = mat3(cos(x1),sin(x1),0.0,-sin(x1),cos(x1),0.0,0.0,0.0,1.0);
    vec3 v = vec3(cos(x2)*sqrt(x3),sin(x2)*sqrt(x3),sqrt(1.0-x3));
    mat3 T = mat3(v.x*v.x,v.x*v.y,v.x*v.z,v.x*v.y,v.y*v.y,v.y*v.z,v.x*v.z,v.y*v.z,v.z*v.z);
    mat3 I = mat3(1.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,1.0);
    mat3 H = I - 2.0*T;
    return -H*R;
}

// 计算OA和OB的夹角
float getAng(vec2 a,vec2 b,vec2 o){
    vec2 oa = normalize(a - o);
    vec2 ob = normalize(b - o);
    float w = acos(dot(oa, ob));
    float p = w / 3.1415926 * 180.0;
    float q = p / 180.0 - 0.5;
    return q;
}


void main(){
    vec2 rotv = aPos.xy * PI2;
    float dx = cos(rotv.x)*cos(rotv.y);
    float dy = sin(rotv.x)*cos(rotv.y);
    float dz = sin(rotv.y);

    vec4 pLT,pLB,pRT,pRB;

    // 如果预设法向量不为0，则使用预设法向量（即旋转结果的法向量为该值）
    if(preNormal.x>0.0||preNormal.y>0.0||preNormal.z>0.0){
        vec3 ds = normalize(preNormal);
        vec3 dv = cross(ds, vec3(dx,dy,dz));
        mat4 rotation = rotationMatrix(ds, 3.1415926/2.0);

        vec4 aLT = vec4(dv, 1.0);
        vec4 aLB = rotationMatrix(ds, 3.1415926/2.0)*vec4(dv, 1.0);
        vec4 aRT = rotationMatrix(ds, 3.1415926/2.0)*rotationMatrix(ds, 3.1415926/2.0)*vec4(dv, 1.0);
        vec4 aRB = rotationMatrix(ds, 3.1415926/2.0)*rotationMatrix(ds, 3.1415926/2.0)*rotationMatrix(ds, 3.1415926/2.0)*vec4(dv, 1.0);

        pLT = aLT + vec4(uTranslation, 0.0);
        pLB = aLB + vec4(uTranslation, 0.0);
        pRT = aRT + vec4(uTranslation, 0.0);
        pRB = aRB + vec4(uTranslation, 0.0);

    }else{
        //否则使用随机旋转
    
        float lb=uRotLb;
        float rb=uRotRb;
        float rotAngle = (aPos.z*(rb-lb)+lb)/180.0*3.1415926;
        vec3 axis = vec3(dx,dy,dz); 

        // 如果旋转轴不为0，则使用旋转轴，否则使用随机旋转轴
        if(uRotAxis.x>1e-2||uRotAxis.y>1e-2||uRotAxis.z>1e-2){
            axis = normalize(uRotAxis);
        }

        
        if(useUniformDist>0.5){
            // 使用均匀分布的随机旋转
            mat3 rot3 = uniformRotationMatrix(aPos.x,aPos.y,aPos.z);
            pLT = vec4(rot3 * vec3(uLT) + uTranslation, 1.0);
            pLB = vec4(rot3 * vec3(uLB) + uTranslation, 1.0);
            pRT = vec4(rot3 * vec3(uRT) + uTranslation, 1.0);
            pRB = vec4(rot3 * vec3(uRB) + uTranslation, 1.0);

        }else{
            // 这是原始的算法
            mat4 rotation = rotationMatrix(axis, rotAngle);
            pLT = rotation * vec4(uLT, 1.0) + vec4(uTranslation, 0.0);
            pLB = rotation * vec4(uLB, 1.0) + vec4(uTranslation, 0.0);
            pRT = rotation * vec4(uRT, 1.0) + vec4(uTranslation, 0.0);
            pRB = rotation * vec4(uRB, 1.0) + vec4(uTranslation, 0.0);
        }
    }


    
    vec4 pts[4];
    pts[0] = pLT;
    pts[1] = pLB;
    pts[2] = pRB;
    pts[3] = pRT;
    int idx = 0;

    // 顶点排序：保证旋转后的四个顶点，投影后，第0个元素为最左上角的顶点
    if(sortVertex>0.5){
        int topYIdx = 0; //Y最大的顶点
        int top2YIdx = 0; //Y第二大的顶点
        for(int i=1;i<4;i++){
            if(pts[i].y/pts[i].z>pts[topYIdx].y/pts[topYIdx].z){
                topYIdx = i;
            }
        }
        top2YIdx = (topYIdx+1)%4;
        for(int i=0;i<4;i++){
            if(i!=topYIdx){
                if(pts[i].y/pts[i].z>pts[top2YIdx].y/pts[top2YIdx].z){
                    top2YIdx = i;
                }
            }
        }

        int leftXIdx = topYIdx; //在Y最大的两个顶点中，X最小的顶点
        int rightXIdx = top2YIdx;
        if(pts[topYIdx].x/pts[topYIdx].z>pts[top2YIdx].x/pts[top2YIdx].z){
            leftXIdx = top2YIdx;
            rightXIdx = topYIdx;
        }


        idx = leftXIdx;
        pLT = pts[idx];
        pLB = pts[(idx+1)%4];
        pRB = pts[(idx+2)%4];
        pRT = pts[(idx+3)%4];
    }
    
    // 法向规范化：保证法向量为逆时针
    if(forceCounterClockwiseNormal>0.5){
        vec2 sLT = pLT.xy / pLT.z;
        vec2 sLB = pLB.xy / pLB.z;
        vec2 sRB = pRB.xy / pRB.z;
        vec2 sRT = pRT.xy / pRT.z;
        vec2 sA = sLB - sLT;
        vec2 sB = sRB - sLB;
        float area = sA.x * sB.y - sA.y * sB.x;
        if(area<0.0){
            vec4 tmp = pLT;
            pLT = pts[idx];
            pLB = pts[(idx+3)%4];
            pRB = pts[(idx+2)%4];
            pRT = pts[(idx+1)%4];
        }
    }


    // 计算扰动后的顶点位置
    vLTd = pLT + vec4(disturbX, disturbY, 0.0, 0.0) * pLT.z / vec4(uFocalX, uFocalY, 1.0, 1.0);
    vLBd = pLB + vec4(disturbX, disturbY, 0.0, 0.0) * pLB.z / vec4(uFocalX, uFocalY, 1.0, 1.0);
    vRTd = pRT + vec4(disturbX, disturbY, 0.0, 0.0) * pRT.z / vec4(uFocalX, uFocalY, 1.0, 1.0);
    vRBd = pRB + vec4(disturbX, disturbY, 0.0, 0.0) * pRB.z / vec4(uFocalX, uFocalY, 1.0, 1.0);

    // 计算在归一化平面(Retina Space)上的位置
    vec4 rLT = vec4(pLT.x / pLT.z * uFocalX, pLT.y / pLT.z * uFocalY, pLT.z, 1.0);
    vec4 rLB = vec4(pLB.x / pLB.z * uFocalX, pLB.y / pLB.z * uFocalY, pLB.z, 1.0);
    vec4 rRT = vec4(pRT.x / pRT.z * uFocalX, pRT.y / pRT.z * uFocalY, pRT.z, 1.0);
    vec4 rRB = vec4(pRB.x / pRB.z * uFocalX, pRB.y / pRB.z * uFocalY, pRB.z, 1.0);

    // 计算顶点内角值
    float iA = getAng(rLB.xy,rRT.xy, rLT.xy);
    float iB = getAng(rLT.xy,rRB.xy, rRT.xy);
    float iC = getAng(rRT.xy,rLB.xy, rRB.xy);


    // 为了上色，需要传递一些变量给片段着色器(shader.frag)
    vec4 v = vec4(iA,iB,iC,1.0);
    vPos = v;
    vCol = vec4(iA,iB,iC,1.0);

    vLT = pLT;
    vLB = pLB;
    vRT = pRT;
    vRB = pRB;

    // 这里处理顶点着色器的输出
    if(outputNormal<0.5){
        // 输出顶点坐标是内角值
        gl_Position = uProj*uModelView*uLocalMat*v;
        gl_PointSize = pSize;
    }else{
        // 输出顶点坐标是法向量
        vec4 edgA = vLT - vRT;
        vec4 edgB = vLB - vRT;
        vec3 n = normalize(cross(edgA.xyz, edgB.xyz));
        gl_Position = uProj*uModelView*uLocalMat*vec4(n, 1.0);
        gl_PointSize = pSize;
    }

}