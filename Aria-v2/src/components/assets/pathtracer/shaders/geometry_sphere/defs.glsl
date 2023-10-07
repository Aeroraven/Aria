struct rts_Sphere{
    vec3 center;
    float radius;
};

void rtf_SphereRayIntersection(rts_Ray r,rts_Sphere sp,inout float sol,inout vec3 normal){
    vec3 center = r.origin-sp.center;
    float polyA = dot(r.direction,r.direction);
    float polyB = 2.0*dot(r.direction,center);
    float polyC = dot(center,center)-sp.radius*sp.radius;
    float delta = polyB*polyB-4.0*polyA*polyC;
    if(delta<0.0){
        sol = -1.0;
        return;
    }
    float sols1 = 0.0,sols2 = 0.0;
    if(abs(delta)<epsf){
        sols1 = -polyB/(2.0*polyA);
        sols2 = sols1;
    }else{
        sols1 = (-polyB-sqrt(delta))/(2.0*polyA);
        sols2 = (-polyB+sqrt(delta))/(2.0*polyA);
    }
    if(sols1<epsf){
        if(sols2<epsf){
            sol = -1.0;
            return;
        }else{
            sol = sols2;
        }
    }else{
        if(sols2<epsf){
            sol = sols1;
        }else{
            sol = min(sols1,sols2);
        }
    }
    normal = normalize(r.origin+r.direction*sol-sp.center);
}