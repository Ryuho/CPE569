#ifndef _UTIL_H_
#define _UTIL_H_
//Do not use a CPP file for this.
//All non-templates must be inline or it will not compile.

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#include <algorithm>
#include <vector>
#include <string>
#include <ostream>
#include <istream>
#include <iostream>
#include <sstream>

namespace util {

   /////////////////// RANDOM ///////////////////
   //////////////////////////////////////////////
   // Returns a random float between 0.0 and 0.9999~
   inline float frand() { 
      return rand() / (float)(RAND_MAX);
   }

   // Returns a random float between min and max inclusive
   inline float frand(float min, float max) { 
      return min + (rand() / (float)RAND_MAX)*(max-min + 1); 
   }

   // Returns a random int between min and max inclusive
   inline int irand(int min, int max) { 
      return min == max ? min :
         min + (rand() % (max-min + 1));
   }

   /////////////////// VECTORS ///////////////////
   ///////////////////////////////////////////////
   template <typename T>
   void removeDuplicates(std::vector<T> &v)
   {
      std::sort(v.begin(), v.end());
      //typename std::vector<T>::iterator newEnd = std::unique(v.begin(), v.end());
      v.resize(std::unique(v.begin(), v.end()) - v.begin());
   }

   //This only removes the first, different than std::remove()
   template <typename T>
   void remove(T t, std::vector<T> &v) {
      for(unsigned i = 0; i < v.size(); i++) {
         if(v[i] == t) {
            v[i] = v.back();
            v.pop_back();
            return;
         }
      }
   }

   template <typename T>
   bool contains(const T &t, std::vector<T> &v) {
      return std::find(v.begin(), v.end(), t) != v.end();
   }

   /////////////////// MATH ///////////////////
   ////////////////////////////////////////////
   template <typename T>
   const T& clamp(T &v, const T &left, const T &right) {
      v = std::min(std::max(v, left), right);
      return v;
   }

   /////////////////// STRING ///////////////////
   //////////////////////////////////////////////
   inline std::vector<std::string> splitString(const std::string &input, 
      const std::string &delim = " ")
   {
	   std::vector<std::string> ret;

	   size_t pos, lastPos = 0;
	   while( ( pos = input.find( delim, lastPos ) ) != std::string::npos ) {
         if (pos != lastPos)
		      ret.push_back( input.substr( lastPos, pos - lastPos ) );
		   lastPos = pos+delim.size();
	   }
	   if( lastPos < input.size() ) 
		   ret.push_back( input.substr( lastPos ) ); 
	   return ret; 
   }

   /////////////////// PRINTING ///////////////////
   ////////////////////////////////////////////////
   template<typename T>
   void vec_print_ptrs(std::ostream& os, std::vector<T*>& v, char* separator="\n")
   {
      typename std::vector<T*>::iterator iter;
      for (iter = v.begin(); iter != (v.end() - 1); iter++ ) {
         os << (**iter) << separator;
      }
      if(v.size() != 0)
         os << (**(v.end() - 1));
   }

   template<typename T>
   void vec_pprint_ptrs(std::ostream& os, std::vector<T*>& v, char* separator="\n")
   {
      os << "[";
      vec_print_ptrs(os, v, separator);
      os << "]";
   }

   template<typename T>
   void vec_print(std::ostream& os, std::vector<T>& v, char* separator="\n")
   {
      typename std::vector<T>::iterator iter;
      for (iter = v.begin(); iter != v.end() - 1; iter++ ) {
         os << *iter << separator;
      }
      if(v.size() != 0)
         os << *(v.end() - 1);
	}

	template<typename T>
	void vec_pprint(std::ostream& os, std::vector<T>& v, char* separator="\n")
	{
		os << "[";
		vec_print(os, v, separator);
		os << "]";
	}
}

/////////////////// ASSERT ///////////////////
//////////////////////////////////////////////
#define ASSERT2(x,y) \
	if(x != y) { \
		std::stringstream ss; \
		ss << __FILE__ << std::endl \
			<< __FUNCTION__ << " (" << __LINE__ << ")" << std::endl \
			<< "Got '" << x << "'. Expecting '" << y << "'."; \
		std::cerr << ss.str().c_str(); \
		throw ss.str().c_str(); \
	}

#define ASSERT(x) \
   if(!x) { \
		std::stringstream ss; \
		ss << __FILE__ << std::endl \
			<< __FUNCTION__ << " (" << __LINE__ << ")" << std::endl \
			<< "Got '" << x << "'." \
		std::cerr << ss.str().c_str(); \
		throw ss.str().c_str(); \
	}

#define STRASSERT(x,y) \
	if(x.compare(y) != 0) { \
		std::stringstream ss; \
		ss << __FILE__ << std::endl \
			<< __FUNCTION__ << " (" << __LINE__ << ")" << std::endl \
			<< "Got '" << x << "'. Expecting '" << y << "'."; \
		std::cerr << ss.str().c_str(); \
		throw ss.str().c_str(); \
	}

#endif //_UTIL_H_

/*
//Tests
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
