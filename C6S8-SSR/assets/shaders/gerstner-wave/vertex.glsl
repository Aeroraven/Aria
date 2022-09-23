#version 300 es
precision highp float;
in vec4 aVert;
in vec2 aTex;
in vec3 aNorm;
in vec3 aTangent;
in vec3 aBitangent;

out vec2 vTex;
out vec3 vNorm;
out vec3 vFragPos;
out vec4 vWorldPos;
out vec3 vTangent;
out vec3 vBitangent;


uniform mat4 uModel;
uniform mat4 uProj;
uniform mat4 uModelIT;
uniform float uTimestamp;

float randomScalar(vec3 worldPos){
    float d = dot(vec3(114.1919, 514.810, 37.719),sin(worldPos));
    return fract(sin(d)*114514.1919810);
}


void waveFunc(vec2 dir, float freq,float ampl, float wavelength, vec4 vo, inout vec4 vertex, inout vec3 tangent,inout vec3 bitangent){
    vec3 vTangentNorm = vec3(0.0,0.0,1.0); //TBN
    float fp = (dot(vo.xz,dir)  + uTimestamp*0.0015)*2.0*3.14159/wavelength;

    vertex.y += ampl*sin(fp);
    vertex.x += ampl*cos(fp)*dir.x;
    vertex.z += ampl*cos(fp)*dir.y;

    vec3 vTx = vec3(1.0-sin(fp)*freq*ampl*dir.x*dir.x, cos(fp)*freq*ampl*dir.x, -dir.y*dir.x*sin(fp)*freq*ampl);
    vec3 vTz = vec3(-sin(fp)*freq*ampl*dir.x*dir.y, cos(fp)*freq*ampl*dir.y, 1.0-dir.y*dir.y*sin(fp)*freq*ampl);

    tangent+=vec3((uModel*vec4(vTx,0.0)).xyz);
    bitangent+=vec3((uModel*vec4(vTz,0.0)).xyz);
}


void main(){
    vec4 vert = aVert;

    float freq = 2.0;
    float ampl = 0.3;
    float wavelength = 5.0;
    
    vBitangent = vec3(0.0);
    vTangent = vec3(0.0);

    for(int i=0;i<8;i++){
        float rnd1 = randomScalar(vec3(995.0+float(i),514.0+float(i),191.0));
        float rnd2 = randomScalar(vec3(144.0*0.9+float(i),514.0*0.7+float(i),151.0));
        float rnd3 = randomScalar(vec3(844.0*0.39+float(i),514.0*0.57+float(i),151.0));
        float rnd4 = randomScalar(vec3(844.0*0.39+float(i*11),514.0*0.57+float(i*45),151.0));
        waveFunc(normalize(vec2(rnd2+rnd4,rnd1+rnd3)),rnd4*1.0,0.1,(rnd1+rnd2+rnd1*rnd2)*2.5,aVert,vert,vTangent,vBitangent);
    }
    vTangent = normalize(vTangent);
    vBitangent = normalize(vBitangent);


    vNorm = cross(vBitangent,vTangent);

    vWorldPos = vert;
    gl_Position = uProj * uModel * vert;
    vFragPos =  (uModel*vert).xyz;
    vTex = aTex;
}