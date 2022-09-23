#version 300 es

precision mediump float;

in mediump vec2 vTex;
uniform sampler2D uSampler;

out vec4 fragmentColor;

void main(){
    float offset = 1.0/300.0;
    float kernel[9] = float[](1.0,2.0,1.0,2.0,4.0,2.0,1.0,2.0,1.0);
    for(int i=0;i<9;i++){
        kernel[i] = kernel[i] / 16.0;
    }
    vec4 wx = vec4(0.0,0.0,0.0,1.0);
    float dx = -offset;
    float dy = -offset;
    for(int i=-1;i<=1;i++){
        for(int j=-1;j<=1;j++){
            wx+=texture(uSampler,vTex+vec2(dx,dy))*kernel[(i+1)*3+(j+1)];
        }
        dx += offset;
        dy += offset;
    }

    fragmentColor = wx;
}