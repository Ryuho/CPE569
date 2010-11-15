#ifndef _UTIL_H_
#define _UTIL_H_
//Do not use a CPP file for this

#include <algorithm>
#include <vector>

namespace util {

   template <typename T>
   void removeDuplicates(std::vector<T> &v)
   {
      std::sort(v.begin(), v.end());
      std::vector<T>::iterator newEnd = std::unique(v.begin(), v.end());
      int newSize = newEnd - v.begin();
      v.resize(newSize);
   }

   //Different than algorithm remove()
   //this only removes the first one, and replaces with the vectors last element
   //algorithm's remove percolates all removed values upward
   template <typename T>
   void remove(T *t, std::vector<T *> &v) {
      for(unsigned i = 0; i < v.size(); i++) {
         if(v[i] == t) {
            v[i] = v.back();
            v.pop_back();
            return;
         }
      }
   }

   template <typename T>
   bool contains(T *t, std::vector<T *> &v) {
      return std::find(v.begin(), v.end(), t) != v.end();
   }

   template <typename T>
   const T& clamp(T &v, const T &left, const T &right) {
      v = std::min(std::max(v, left), right);
      return v;
   }
}

#endif //_UTIL_H_

/*
#include <assert.h>

void removeDuplicatesTest() {
   int arr[] = {0, 5, 5, 3, 2, 5, 0, 10, 8, 7, 5, 4, 6};

   std::vector<int> v;
   unsigned size = sizeof(arr)/4;
   //printf("size=%d\n", size);
   for(unsigned i = 0; i < size; i++) {
      v.push_back(arr[i]);
   }
   printf("test1\n");
   printf("size=%d\n", v.size());
   for(unsigned i = 0; i < size; i++) {
      printf("%d ", v[i]);
   }
   printf("test2\n");
   removeDuplicates(v);
   printf("size=%d\n", v.size());
   for(unsigned i = 0; i < v.size(); i++) {
      printf("%d ", v[i]);
   }
}

void clampTest() {
   int x, y;

   x = y = 4;
   y = clamp(x, 5, 10);
   assert(x == 5 && y == 5);

   x = y = 5;
   y = clamp(x, 5, 10);
   assert(x == 5 && y == 5);

   x = y = 11;
   y = clamp(x, 5, 10);
   assert(x == 10 && y == 10);

   x = y = 10;
   y = clamp(x, 5, 10);
   assert(x == 10 && y == 10);

   x = y = 8;
   y = clamp(x, 5, 10);
   assert(x == 8 && y == 8);
}

void removeTest() {
   int arr[] = {0, 5, 5, 3, 2, 5, 0, 10, 8, 7, 5, 4, 6};
   std::vector<int *> v;
   unsigned size = sizeof(arr)/4;
   for(unsigned i = 0; i < size; i++) {
      v.push_back((int *)arr[i]);
   }

   size = v.size();
   remove((int*)-1, v);
   assert(v.size() == size);
   remove((int*)50, v);
   assert(v.size() == size);
   remove((int*)25, v);
   assert(v.size() == size);

   size = v.size();
   remove((int*)0, v);
   assert(v.size() == size - 1);

   size = v.size();
   remove((int*)0, v);
   assert(v.size() == size - 1);

   size = v.size();
   remove((int*)0, v);
   assert(v.size() == size);

   size = v.size();
   remove((int*)6, v);
   assert(v.size() == size - 1);

   size = v.size();
   remove((int*)0, v);
   assert(v.size() == size);

   size = v.size();
   remove((int*)6, v);
   assert(v.size() == size);

   printf("size=%d\n", v.size());
   for(unsigned i = 0; i < v.size(); i++) {
      printf("%d ", v[i]);
   }
}

void containsTest() {
   int arr[] = {0, 5, 5, 3, 2, 5, 0, 10, 8, 7, 5, 4, 6};
   std::vector<int *> v;
   unsigned size = sizeof(arr)/4;
   for(unsigned i = 0; i < size; i++) {
      v.push_back((int *)arr[i]);
   }
   size = v.size();

   assert(contains((int *) 0, v));
   assert(contains((int *) 5, v));
   assert(contains((int *) 3, v));
   assert(contains((int *) 2, v));
   assert(contains((int *) 6, v));
   assert(contains((int *) 8, v));

   assert(!contains((int *) -1, v));
   assert(!contains((int *) -11, v));
   assert(!contains((int *) -9, v));
   assert(!contains((int *) -100, v));
   assert(!contains((int *) 50, v));
}

void testAssertFail() {
   assert(false); //shows that assert is working
}

int main()
{
   containsTest();

   printf("tests complete\n");
}
*/