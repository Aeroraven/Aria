struct Camera
{
    float4x4 proj;
    float4x4 view;
    float4x4 model;
    float4 camPos;
    float4 timer; //Tick,0,0,0
};
