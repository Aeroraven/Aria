struct VSOutput{
    [[vk::location(0)]] float2 texCoord : TEXCOORD0;
};

Texture2D texEdge : register(t0, space1);
SamplerState sampEdge : register(s0, space1);

Texture2D texTemplate : register(t0, space0);
SamplerState sampTemplate : register(s0, space0);

static const int SEARCH_BOUND = 16;
static const float EPS = 1e-5;

int getBoundType(float top, float bottom){
    if (top > EPS && bottom > EPS){
        return 3;
    }else if (top > EPS){
        return 1;
    }else if (bottom > EPS){
        return 2;
    }
    return 0;
}

float4 main(VSOutput vsOut) : SV_TARGET0{
    
    float texH, texW, texL;
    texEdge.GetDimensions(0, texW, texH, texL);
    float dx = 1 / texW, dy = 1 / texH;
    float2 texc = vsOut.texCoord;
    
    //Search Blend Factor To/From Top
    int  hL = 0, hR = 0;
    for (hL = 1; hL <= SEARCH_BOUND; hL += 1){
        float2 dev = float2(-dx * float(hL), 0);
        if (texEdge.Sample(sampEdge, texc + dev).b < EPS){
            break;
        }
    }
    for (hR = 1; hR <= SEARCH_BOUND; hR += 1){
        float2 dev = float2(dx * float(hR), 0);
        if (texEdge.Sample(sampEdge, texc + dev).b < EPS){
            break;
        }
    }
    hL--;
    float hLTop = texEdge.Sample(sampEdge, texc + float2(float(hL) * (-dx), -dy)).r;
    float hLBottom = texEdge.Sample(sampEdge, texc + float2(float(hL) * (-dx), dy)).r;
    float hRTop = texEdge.Sample(sampEdge, texc + float2(float(hR) * (+dx), -dy)).r;
    float hRBottom = texEdge.Sample(sampEdge, texc + float2(float(hR) * (+dx), dy)).r;
    int leftTp = getBoundType(hLTop, hLBottom);
    int rightTp = getBoundType(hRTop, hRBottom);
    
    float leftSampOff = float(SEARCH_BOUND * leftTp + hL) / 63.0;
    float rightSampOff = float(SEARCH_BOUND * rightTp + (hR - 1)) / 63.0;
    float2 udBlendFactor = texTemplate.Sample(sampTemplate, float2(leftSampOff, rightSampOff)).xy;
    
    //Search Blend Factor To/From Left
    int vL = 0, vR = 0;
    for (vL = 1; vL <= SEARCH_BOUND; vL += 1){
        float2 dev = float2(0, -dy * float(vL));
        if (texEdge.Sample(sampEdge, texc + dev).r < EPS){
            break;
        }
    }
    for (vR = 1; vR <= SEARCH_BOUND; vR += 1){
        float2 dev = float2(0, dy * float(vR));
        if (texEdge.Sample(sampEdge, texc + dev).r < EPS){
            break;
        }
    }
    vL--;
    float vRTop = texEdge.Sample(sampEdge, texc + float2(-dx,float(vR) * (+dy))).b;
    float vRBottom = texEdge.Sample(sampEdge, texc + float2(0, float(vR) * (+dy))).b;
    float vLTop = texEdge.Sample(sampEdge, texc + float2(-dx, float(vL) * (-dy))).b;
    float vLBottom = texEdge.Sample(sampEdge, texc + float2(0, float(vL) * (-dy))).b;
    int vLeftTp = getBoundType(vRTop, vRBottom);
    int vRightTp = getBoundType(vLTop, vLBottom);
    
    float vLeftSampOff = float(SEARCH_BOUND * vLeftTp + (vR - 1)) / 63.0;
    float vRightSampOff = float(SEARCH_BOUND * vRightTp + vL) / 63.0;
    float2 lrBlendFactor = texTemplate.Sample(sampTemplate, float2(vLeftSampOff, vRightSampOff)).xy;
    
    // R: Cur<-Top // G: Cur->Top // B: Cur<-Left // A: Cur->Left
    return float4(udBlendFactor, lrBlendFactor);
}
