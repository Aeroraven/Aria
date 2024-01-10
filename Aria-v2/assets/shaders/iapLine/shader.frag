#version 300 es
precision mediump float;
out vec4 FragColor;
in vec3 vCol;
void main(){
    FragColor = vec4(vCol, 1.0);
}