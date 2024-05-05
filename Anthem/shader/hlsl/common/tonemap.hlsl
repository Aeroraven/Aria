#ifndef AT_H_TONEMAP
#define AT_H_TONEMAP

// GT Tonemap From: https://www.desmos.com/calculator/gslcdxvipg

float4 gtTonemapW(float4 x, float4 e0, float4 e1)
{
    float4 a = (x - e0) / (e1 - e0);
    return clamp(a * a * (3 - 2 * a), float4(0, 0, 0, 0), float4(1, 1, 1, 1));
}
float4 gtTonemapH(float4 x, float4 e0, float e1)
{
    return clamp((x - e0) / (e1 - e0), float4(0, 0, 0, 0), float4(1, 1, 1, 1));
}

float4 gtTonemapping(float4 x, float maxBrightness=1.0,float contrast=1,float linearSecStart=0.22,
    float linSecLength = 0.4, float blackTightnessC = 1.33, float blackTightnessB = 0.0)
{
    float l0 = (maxBrightness - linearSecStart) / contrast;
    float L0 = linearSecStart - l0 / contrast;
    float L1 = L0 + 1 / contrast;
    // L(x) = linearSecStart + contrast(x-linearSecStart)
    // T(x) = linearSecStart*pow(x/linearSecStart,c)+b
    float S0 = linearSecStart + l0;
    float S1 = linearSecStart + contrast * l0;
    // S(x) = maxBrightness - (maxBrightness-S1)*exp(-C2(x-S0)/maxBrightness)
    float C2 = (contrast * maxBrightness) / (maxBrightness - S1);
    // w0(x) = 1-w(0,x,contrast)
    // w2(x) = max(0,sgn(x-m+l0))
    // w1(x) = 1-w0(x)-w2(x)
    
    float4 T = linearSecStart * pow(x / linearSecStart, blackTightnessC) + blackTightnessB;
    float4 w0 = 1 - gtTonemapW(x, 0, contrast);
    float4 L = linearSecStart + contrast * (x - linearSecStart);
    float4 w2 = max(0, sign(x - linearSecStart - l0));
    float4 w1 = 1 - w0 - w2;
    float4 S = maxBrightness - (maxBrightness - S1) * exp(-C2 * (x - S0) / maxBrightness);
    float4 blended = T * w0 + L * w1 + S * w2;
    return blended;
}

#endif // AT_H_TONEMAP