struct FluidParams{
    float4 params;
    int4 grid;
};

void jacobiIter(
    RWTexture2D<float4> x,
    RWTexture2D<float4> b,
    uint2 uv,
    float alpha,
    float rbeta,
    out float4 outX
){
    // (xL+xR+xT+xB)+aB=bX
    // X = ((xL+xR+xT+xB)+aB)/b

    float4 xL = x[uv-uint2(1,0)];
    float4 xR = x[uv+uint2(1,0)];
    float4 xT = x[uv-uint2(0,1)];
    float4 xB = x[uv+uint2(0,1)];
    float4 xC = x[uv];
    float4 vB = b[uv];
    outX = (xL+xR+xT+xB+alpha*vB)*rbeta;
}

void jacobiIterDivergence(
    RWTexture2D<float4> x,
    RWTexture2D<float4> b,
    uint2 uv,
    float alpha,
    float rbeta,
    out float4 outX
){
    // (xL+xR+xT+xB)+aB=bX
    // X = ((xL+xR+xT+xB)+aB)/b

    float4 xL = x[uv-uint2(1,0)];
    float4 xR = x[uv+uint2(1,0)];
    float4 xT = x[uv-uint2(0,1)];
    float4 xB = x[uv+uint2(0,1)];
    float4 xC = x[uv];
    
    // Divergence of b
    float4 bL = b[uv-uint2(1,0)];
    float4 bR = b[uv+uint2(1,0)];
    float4 bT = b[uv-uint2(0,1)];
    float4 bB = b[uv+uint2(0,1)];
    float4 bX = (bR-bL)*0.5;
    float4 bY = (bT-bB)*0.5;
    float4 vB = bX+bY;
    outX = (xL+xR+xT+xB+alpha*vB)*rbeta;
}