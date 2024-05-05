#ifndef AT_H_RANDOM
#define AT_H_RANDOM

float trivialRandom(float2 x) {
    return frac(sin(dot(x, float2(12.9898, 4.1414))) * 43758.5453);
}

#endif // AT_H_RANDOM