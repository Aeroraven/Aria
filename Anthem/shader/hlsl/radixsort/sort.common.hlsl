#ifndef AT_RADIXSORT_H
#define AT_RADIXSORT_H

struct Uniforms{
    uint4 workGroupSize;
    uint4 parallelBits; // X: Cur Iteration
    uint4 sequenceLength; 
};

#endif // AT_RADIXSORT_H
