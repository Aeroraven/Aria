// Computation Operators

// Radix Sort & Parallel Scanning

template<typename T>
void scanReduce(in T src,out T dst, int pos, int depth){
    // Lv0: A[1]=A[1]+A[0], A[3]=A[3]+A[2], ...
    // Lv1: A[3]=A[3]+A[1], A[7]=A[7]+A[5], ...

    int delta = (1<<depth);
    int first = (1 << (depth+1))-1;
    int increment = (1<<(depth+1));

    int realPos = first + pos*increment;
    dst[realPos] = src[realPos] + src[realPos-delta];
}

template<typename T>
void scanAccumulate(in T src,out T dst, int pos, int depth){
    // Lv0: A[2]=A[2]+A[1], A[4]=A[4]+A[3], A[6]=A[6]+A[5], ...
    // Lv1: A[5]=A[5]+A[3], A[9]=A[9]+A[7], A[13]=A[13]+A[11], ...

    int delta = (1<<depth);
    int increment = (1<<(depth+1));
    int first = (1 << (depth+1))-1 + delta;
    int realPos = first + pos*increment;
    dst[realPos] = src[realPos] + src[realPos-delta];
}

