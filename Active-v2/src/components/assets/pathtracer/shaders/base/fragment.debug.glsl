#version 300 es
precision mediump float;

in vec2 vRayDirection;
in vec3 vOriginPos;
out vec4 fragmentColor;

const float epsf = 1e-6;

struct rts_Ray{
    vec4 origin;
    vec4 direction;
};

struct rts_Plane{
    vec4 normal;
    vec4 point;
};
struct rts_Sphere{
    vec4 center;
    float radius;
};
float rtf_PlaneRayIntersection(rts_Ray r,rts_Plane sp){
    float t = dot(sp.point-r.origin,sp.normal)/dot(r.direction,sp.normal);
    if(t<0.0){
        return -1.0;
    }else{
        return t;
    }
}
float rtf_SphereRayIntersection(rts_Ray r,rts_Sphere sp){
    //(ox+t*rx-cx)^2+(oy+t*ry-cy)^2+(oz+t*rz-cz)^2=d^2
    //(rx^2+ry^2+rz^2)*t^2+2(rx*cx+ry*cy+rz*cz)*t+(cz^2+cx^2+cy^2-d^2)=0
    vec4 center = r.origin-sp.center;
    float polyA = dot(r.direction,r.direction);
    float polyB = 2.0*dot(r.direction,center);
    float polyC = dot(center,center)-sp.radius*sp.radius;
    float delta = polyB*polyB-4.0*polyA*polyC;
    if(delta<0.0){
        return -1.0;
    }
    float sols1 = 0.0,sols2 = 0.0;
    if(abs(delta)<epsf){
        sols1 = -polyB/(2.0*polyA);
        sols2 = sols1;
    }else{
        sols1 = (-polyB-sqrt(delta))/(2.0*polyA);
        sols2 = (-polyB+sqrt(delta))/(2.0*polyA);
    }
    if(sols1<0.0){
        if(sols2<0.0){
            return -1.0;
        }else{
            return sols2;
        }
    }else{
        if(sols2<0.0){
            return sols1;
        }else{
            return min(sols1,sols2);
        }
    }
}

vec4 hitTest(rts_Ray r){
    float bestT = 1e80;

    if(false){
        rts_Sphere sp;
        sp.center = vec4(0.0,0.0,12.0,1.0);
        sp.radius = 1.0;
        float t = rtf_SphereRayIntersection(r,sp);
        
        if(t<bestT&&t>=-epsf){
            bestT = t;
        }
    }

    if(false){
        rts_Plane sp;
        sp.normal = vec4(0.0,1.0,0.0,0.0);
        sp.point = vec4(0.0,-4.0,0.0,1.0);
        float t = rtf_PlaneRayIntersection(r,sp);
        if(t<bestT&&t>=-epsf){
            bestT = t;
        }
    }

    if(bestT<1e10){
        return vec4(1.0,0.0,0.0,1.0);
    }else{
        return vec4(0.0,0.0,0.0,1.0);
    }
    
}

void main(){
    vec3 normalizedRayDir = normalize(vec3(vRayDirection,1.0));
    rts_Ray r;
    r.origin = vec4(vOriginPos,1.0);
    r.direction = vec4(normalizedRayDir,0.0);
    fragmentColor = hitTest(r);
}