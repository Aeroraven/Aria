#version 300 es
precision highp float;

uniform mat4 uProj;
uniform mat4 uViewOrtho;
uniform mat4 uModelView;
uniform mat4 uLocalMat;
in vec3 aPos;

uniform vec3 uLT;
uniform vec3 uLB;
uniform vec3 uRT;
uniform vec3 uRB;

uniform float uFocalX;
uniform float uFocalY;
uniform vec3 uTranslation;

uniform float uRotLb;
uniform float uRotRb;
uniform vec3 uRotAxis;
uniform float pSize;
out vec4 vLT;
out vec4 vLB;
out vec4 vRT;
out vec4 vRB;
out vec4 vPos;
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

float getAng(vec2 a,vec2 b,vec2 o){
    vec2 oa = normalize(a - o);
    vec2 ob = normalize(b - o);
    float w = acos(dot(oa, ob));
    float p = w / 3.1415926 * 180.0;
    float q = p / 180.0 - 0.5;
    return q;
}

void main(){
    vec2 rotv = aPos.xy;
    float dx = cos(rotv.x)*cos(rotv.y);
    float dy = sin(rotv.x)*cos(rotv.y);
    float dz = sin(rotv.y);

    float lb=uRotLb;
    float rb=uRotRb;
    float rotAngle = (aPos.z*(rb-lb)+lb)/180.0*3.1415926;
    vec3 axis = vec3(dx,dy,dz); 

    if(uRotAxis.x>1e-2||uRotAxis.y>1e-2||uRotAxis.z>1e-2){
        axis = normalize(uRotAxis);
    }

    mat4 rotation = rotationMatrix(axis, rotAngle);

    vec4 pLT = rotation * vec4(uLT, 1.0) + vec4(uTranslation, 0.0);
    vec4 pLB = rotation * vec4(uLB, 1.0) + vec4(uTranslation, 0.0);
    vec4 pRT = rotation * vec4(uRT, 1.0) + vec4(uTranslation, 0.0);
    vec4 pRB = rotation * vec4(uRB, 1.0) + vec4(uTranslation, 0.0);

    vec4 rLT = vec4(pLT.x / pLT.z * uFocalX, pLT.y / pLT.z * uFocalY, pLT.z, 1.0);
    vec4 rLB = vec4(pLB.x / pLB.z * uFocalX, pLB.y / pLB.z * uFocalY, pLB.z, 1.0);
    vec4 rRT = vec4(pRT.x / pRT.z * uFocalX, pRT.y / pRT.z * uFocalY, pRT.z, 1.0);
    vec4 rRB = vec4(pRB.x / pRB.z * uFocalX, pRB.y / pRB.z * uFocalY, pRB.z, 1.0);

    float iA = getAng(rLB.xy,rRT.xy, rLT.xy);
    float iB = getAng(rLT.xy,rRB.xy, rRT.xy);
    float iC = getAng(rRT.xy,rLB.xy, rRB.xy);

    vec4 v = vec4(iA,iB,iC,1.0);
    vPos = v;
    gl_Position = uProj*uModelView*uLocalMat*v;
    gl_PointSize = pSize;

    vLT = rLT;
    vLB = rLB;
    vRT = rRT;
    vRB = rRB;
}