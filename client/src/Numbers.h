#ifndef _RANDOM_H_
#define _RANDOM_H_

#include <algorithm>

inline float rval()
{
   return rand()/(float)RAND_MAX;
}

inline float nrval()
{
   return rval()*2 - 1;
}

inline int rint(int maxval)
{
   return rand() % (maxval+1);
}

// modulo that handles negatives the way you'd want
inline int pmod(int lhs, int rhs)
{
   return ((lhs%rhs)+rhs)%rhs;
}

template <typename T>
inline T range(T low, T val, T high)
{
   return max(low, min(val, high));
}

inline int modDist(int v1, int v2, int base)
{
   return pmod(min(abs(v1-v2), base-abs(v1-v2)), base);
}

inline int log2(int n)
{
   n = abs(n);
   int ret = 0;
   while (n >>= 1)
      ret++;
   return ret;
}

#endif