const int RTM_Specular=2;
float RTMC_Specular_FuzzyParam=0.0;

void rtf_reflectSpecular(inout vec3 hitNorm,inout vec3 hitPoint,inout vec3 inRayDir, inout vec4 emissionColor,inout vec4 materialColor,inout vec3 outRayDir){
    vec3 reflectDir = reflect(inRayDir, hitNorm);
    vec3 fuzzyDir = rtf_randomUnitVector();
    outRayDir = normalize(normalize(reflectDir)+RTMC_Specular_FuzzyParam*fuzzyDir);
    materialColor = rfMaterialColor;
    emissionColor = rfEmissionColor;
    
}