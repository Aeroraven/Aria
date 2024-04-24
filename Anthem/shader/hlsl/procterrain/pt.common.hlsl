struct Camera
{
    float4x4 proj;
    float4x4 view;
    float4x4 model;
    float4 camPos;
    float4 timer; //Tick,0,0,0
};

static const float GRID_SIZE = 96.0;
static const float GRID_SIZE_Y = 144.0;
static const float COORDINATE_SCALE = 256.0;
static const float Y_ELEVATION = 829.0;