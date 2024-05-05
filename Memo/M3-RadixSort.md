## Radix Sort

Sort a sequence $A[i]$



### 1. Trivial Radix Sort (Intra Block)

Algorithm steps:

- For a desired bit $k$ and $A[i,k]=(A[i]>>k)\&1$, the algorithm checks whether it's zero. $F[i,k]=1-A[i,k]$. 

- Scan the sequence $F$ to get the prefix sum sequence $S[i,k]=sum(F[0...i-1,k])$. Let $T$ denote $S[n,k]$

- For a desired position $i$, the algorithm calculates the position $P$
  $$
  P[i,k]=(!A[i,k])?S[i,k]:(T+i-S[i,k])
  $$

- Scatter using $A,P$ and proceed to the next bit $k+1$

Example:

| A    | 1    | 0    | 1    | 1    | 0    | 0    | 1    | 1    |
| ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- |
| F    | 0    | 1    | 0    | 0    | 1    | 1    | 0    | 0    |
| S    | 0    | 0    | 1    | 1    | 1    | 2    | 3    | 3    |
| P    | 3    | 0    | 4    | 5    | 1    | 2    | 6    | 7    |



#### 1.1. Prefix Sum / Scan

For a sequence $W[i]$,  prefix sum $V[i]=sum(W[0...i])$

**Reduce Step**: Results after reduction step
$$
V[i]=sum(V[i-lowbit(i+1)+1],V[i])
$$
At each step $d$, set $W[k\times 2^d-1]\gets W[k\times 2^d-1]+W[(k-1)\times 2^d-1]$

**Scan Step**: 

At each step $d$, set $W[k\times 2^{d+1}-1+2^d]\gets W[k\times 2^{d+1}-1+2^d]+W[k\times 2^{d+1}-1]$



### 2. Radix Sort in Multiple Blocks

Sort a sequence $A[i]$, with block size $B$ and bit size $N$. Algorithm steps:

- For each block $b$, calculate local prefix sum $Pr[b,c,i]$ for possible bit candidates $c$
- Write counters into histogram $H\in R^{C\times D}$, where $D$ is the number of blocks, $C$ is the number of bit candidates.
- Perform scan algorithm on $H$ to get prefix sum sequence $Q$
- Scatter using $A,Q$ and proceed to next bit set $b+N$



### 3.  Invocations and Work Groups

**Concepts**:

- Local Work Group:  data that runs on a single compute unit

- Sub Group: Specification

  - > A set of  invocations that can synchronize and share data with each other efficiently. In CS, the local workgroup is a superset of the subgroup.
    >
    > For many implementations, a subgroup is the groups of invocations that run the same instruction at once. Subgroups allow for a shader writer to work at a finer granularity than a single workgroup.

  - Inactive invocations: when work group size that is less than the subgroup size







