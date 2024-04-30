struct FluidParams{
    float4 params;
    int4 grid;
    float4 splat;
    float4 window;
    float4 dye; //Decay
};

void jacobiIter(
    RWTexture2D<float4> x,
    RWTexture2D<float4> b,
    int2 uv,
    float alpha,
    float rbeta,
    out float4 outX
){
    // (xL+xR+xT+xB)+aB=bX
    // X = ((xL+xR+xT+xB)+aB)/b

    float4 xL = x[uv-int2(1,0)];
    float4 xR = x[uv+int2(1,0)];
    float4 xT = x[uv-int2(0,1)];
    float4 xB = x[uv+int2(0,1)];
    float4 xC = x[uv];
    float4 vB = b[uv];
    outX = (xL+xR+xT+xB+alpha*vB)*rbeta;
}

float4 fetchX(
    RWTexture2D<float4> x,
    int2 uv,
    int2 grid
){
    float4 outX;
    if(uv.x<=0||uv.y<=0||uv.x>=grid.x-1||uv.y>=grid.y-1){
        float offsetX = 0;
        float offsetY = 0;
        if(uv.x<=0){
            offsetX = 1;
        }
        if(uv.y<=0){
            offsetY = 1;
        }
        if(uv.x>=grid.x-1){
            offsetX = -1;
        }
        if(uv.y>=grid.y-1){
            offsetY = -1;
        }
        outX = x[uv+int2(offsetX, offsetY)];
        return outX;
    }
    outX = x[uv];
    return outX;
}

float4 fetchU(
    RWTexture2D<float4> u,
    int2 uv,
    int2 grid
){
    float4 outU;
    if(uv.x<=0||uv.y<=0||uv.x>=grid.x-1||uv.y>=grid.y-1){
        float offsetX = 0;
        float offsetY = 0;
        if(uv.x<=0){
            offsetX = 1;
        }
        if(uv.y<=0){
            offsetY = 1;
        }
        if(uv.x>=grid.x-1){
            offsetX = -1;
        }
        if(uv.y>=grid.y-1){
            offsetY = -1;
        }
        outU = -u[uv+int2(offsetX, offsetY)];
        return outU;
    }
    outU = u[uv];
    return outU;
}

void jacobiIterDivergence(
    RWTexture2D<float4> x,
    RWTexture2D<float4> b,
    int2 uv,
    float alpha,
    float rbeta,
    int2 grid,
    out float outX
){
    // (xL+xR+xT+xB)+aB=bX
    // X = ((xL+xR+xT+xB)+aB)/b

    float xL = fetchX(x, uv-int2(1,0), grid).r;
    float xR = fetchX(x, uv+int2(1,0), grid).r;
    float xT = fetchX(x, uv-int2(0,1), grid).r;
    float xB = fetchX(x, uv+int2(0,1), grid).r;
    float xC = x[uv].r;
    
    // Divergence of b
    float4 bL = fetchU(b, uv-int2(1,0), grid);
    float4 bR = fetchU(b, uv+int2(1,0), grid);
    float4 bT = fetchU(b, uv-int2(0,1), grid);
    float4 bB = fetchU(b, uv+int2(0,1), grid);
    float bX = ((bR-bL)*0.5).x;
    float bY = ((bB-bT)*0.5).y;
    float vB = bX+bY;
    outX = (xL+xR+xT+xB+alpha*vB)*rbeta;
}