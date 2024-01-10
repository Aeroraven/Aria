#version 300 es
precision highp float;
//--------------------------------
// Varying 变量（这些变量值由shader.vert传出）
//--------------------------------
in vec4 vLT;
in vec4 vLB;
in vec4 vRT;
in vec4 vRB;

in vec4 vLTd;
in vec4 vLBd;
in vec4 vRTd;
in vec4 vRBd;

in vec4 vCol;
in vec4 vPos;

// 输出到片元着色器的变量
out vec4 fDiff;

//--------------------------------
// Uniform 变量 (这些变量在AriaStageInternalAngleReproj.ts中控制)
//--------------------------------
// 用于计算角度的参考向量（Z轴正方向）
uniform vec3 uRef;

// ColorMap（详细见AriaStageInternalAngleReproj.ts）
uniform float useDot;

// 顶点着色器的输出是否是法线
uniform float outputNormal;

void main(){
    // 计算两条邻边
    vec4 edgA = vLT - vRT;
    vec4 edgB = vLB - vRT;

    vec4 edgAd = vLTd - vRTd;
    vec4 edgBd = vLBd - vRTd;

    // 计算法线
    vec3 n = normalize(cross(edgA.xyz, edgB.xyz));
    vec3 nd = normalize(cross(edgAd.xyz, edgBd.xyz));

    // 计算和参考向量的夹角
    float d = abs(dot(n,normalize(uRef.xyz)));
    float ac = acos(d)/3.1415926*2.0;

    // 计算法线和扰动后法向的夹角
    float ndiff = dot(n,nd);


    if(outputNormal>0.5){
        // 如果顶点着色器的输出是法线，那么直接输出内角值作为颜色
        fDiff = vec4(vCol.xyz*0.5+0.5,1.0);
        return;
    }

    if(useDot >0.9){
        // 使用法向绝对值作为颜色
        fDiff = vec4(abs(n),1.0);
    }else if(useDot >0.7){
        // 使用扰动后法向夹角作为颜色
        fDiff = vec4(ndiff,1.0-ndiff,0.0,1.0);
    }
    else if(useDot >0.5){
        // 使用和参考向量的夹角作为颜色
        fDiff = vec4(ac,1.0-ac,0.0,1.0);
    }
    else if(useDot >0.2){
        // 使用顶点着色器输出的顶点位置作为颜色
        fDiff = vPos*0.5+0.5;
    }else{
        // 使用法向作为颜色
        n.xy = abs(n.xy);
        if(dot(n,normalize(uRef.xyz))<0.0){
            fDiff = vec4(n,1.0);
        }else{
            fDiff = vec4(n,1.0);
        }
    }
    
}