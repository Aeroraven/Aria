const int RTM_Lambertian=1;
void rtf_reflectLambertian(inout vec3 hitNorm,inout vec3 hitPoint,inout vec3 inRayDir, inout vec4 emissionColor,inout vec4 materialColor,inout vec3 outRayDir){
    vec3 reflDir = rtf_randomUnitVector(hitNorm+gl_FragCoord.xyz+hitPoint);
    float simOut = dot(hitNorm,reflDir)/length(reflDir)/length(hitNorm); 
    if(simOut < 0.0){
        reflDir = -reflDir;
        simOut = -simOut;
    }
    outRayDir = reflDir;
    materialColor = vec4(1.0,0.0,0.0,1.0) * simOut;
    emissionColor = vec4(0.0,0.0,0.0,1.0) * simOut;
}