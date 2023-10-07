const int RTM_Refractive=3;

void rtf_reflectRefractive(inout vec3 hitNorm,inout vec3 hitPoint,inout vec3 inRayDir, inout vec4 emissionColor,inout vec4 materialColor,inout vec3 outRayDir){
    vec3 reflectDir = vec3(1.0);
    float eta = 1.0/1.5;
    
    if(dot(hitNorm,inRayDir) >= 0.0){
        reflectDir = refract(inRayDir, -hitNorm,1.0/eta);
    }else{
        reflectDir = refract(inRayDir, hitNorm,eta);
    }
    outRayDir = normalize(reflectDir);
    materialColor = rfMaterialColor;
    emissionColor = rfEmissionColor;
}