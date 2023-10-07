#version 300 es
precision highp float;
uniform sampler2D uSourceFrame;
uniform sampler2D uDownsample;
uniform int uLod;
out vec4 fragmentColor;
in vec2 vTex;

vec4 gaussian5A(){
    //Prev Mip = SourceFrame
    ivec2 texSize = textureSize(uSourceFrame, 0);
    vec2 texOffset = 1.0 / vec2(float(texSize.x),float(texSize.y));

    float k[5];
    float sum = 0.0;
    for(int i=-2;i<=2;i++){
        k[i+2] = exp(-pow(float(i),2.0)/2.0);
    }
    vec4 color = vec4(0.0);
    
    for(int i=-2;i<=2;i++){
        for(int j=-2;j<=2;j++){
            float w = k[i+2]*k[j+2];
            sum+=w;
            color += w * texture(uSourceFrame,vTex+texOffset*vec2(float(i),float(j)));
        }
    }
    return color/sum;
}

vec4 gaussian5B(){
    //Curr Mip = uDownSample(LOD)
    ivec2 texSize = textureSize(uDownsample, 0);
    vec2 texOffset = 1.0 / vec2(float(texSize.x),float(texSize.y));

    float k[5];
    int r=2;
    float sum = 0.0;
    for(int i=-r;i<=r;i++){
        k[i+2] = exp(-pow(float(i),2.0)/2.0);
    }
    vec4 color = vec4(0.0);
    for(int i=-r;i<=r;i++){
        for(int j=-r;j<=r;j++){
            float w = k[i+2]*k[j+2];
            sum+=w;
            color += w * texture(uDownsample,vTex+texOffset*vec2(float(i),float(j)));
        }
    }
    return color/sum;
}
void main(){
    fragmentColor = gaussian5B()+gaussian5A();   
}