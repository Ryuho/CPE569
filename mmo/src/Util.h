#ifndef _UTIL_H_
#define _UTIL_H_
//Do not use a CPP file for this

#include <algorithm>
#include <vector>

namespace util {

   template <typename T>
   void removeDuplicates(std::vector<T> items)
   {
      std::sort(items.begin(), items.end());
      std::unique(items.begin(), items.end());
   }

   //Different than algorithm remove()
   //this only removes the first one, and replaces with the vectors last element
   //algorithm's remove percolates all removed values upward
   template <typename T>
   void remove(T *t, std::vector<T *> v) {
      for(unsigned i = 0; i < v.size(); i++) {
         if(v[i] == t) {
            v[i] = v.back();
            v.pop_back();
            return;
         }
      }
   }
}

#endif //_UTIL_H_