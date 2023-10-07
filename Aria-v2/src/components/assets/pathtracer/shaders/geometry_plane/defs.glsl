struct rts_Plane{
    vec3 normal;
    vec3 point;
};

void rtf_PlaneRayIntersection(rts_Ray r,rts_Plane sp,inout float sol){
    float t = dot(sp.point-r.origin,sp.normal)/dot(r.direction,sp.normal);
    if(t<0.0){
        sol = -1.0;
    }else{
        sol = t;
    }
}