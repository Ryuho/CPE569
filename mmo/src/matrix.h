// Copyright (c) Bill Hess 2010.
//
// Use, modification and distribution are subject to the
// Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt).
//
// 

#ifndef _MATRIX_H_
#define _MATRIX_H_

#include <cmath>
#include <cassert>
#include <algorithm>

namespace mat {

// ***************************
//   Matrix Utilities
// ***************************

template<typename matrix, int N>
struct DecompositionT {
   matrix L, U;
   int P[N];
   bool valid;
};

template <typename U>
void swap(U &lhs, U &rhs) {
   U temp = lhs;
   lhs = rhs;
   rhs = temp;
}

inline int mod(int i, int j)
{
   return (i%j+j)%j;
}

inline bool floatCompare(float f1, float f2, int maxDist = 4)
{
   int i1 = *(int*)&f1, i2 = *(int*)&f2;

   if (i1 < 0)
      i1 = 0x80000000 - i1;
   if (i2 < 0)
      i2 = 0x80000000 - i2;

   return abs(i1 - i2) <= maxDist;
}

// ***************************
//   Member Implementations
// ***************************

// CRTP helpers
#define _self static_cast<T*>(this)
#define _self_const static_cast<const T*>(this)


template <typename T, int rows, int cols, typename Transpose_Type>
struct matrix_impl {

   matrix_impl() {
      for (int i = 0; i < cols; i++)
         for (int j = 0; j < rows; j++)
            _self->m[i][j] = 0.0f;
   }

   matrix_impl(float val) {
      for (int i = 0; i < cols; i++)
         for (int j = 0; j < rows; j++)
            _self->m[i][j] = val;
   }

   Transpose_Type transpose() {
      Transpose_Type ret;

      for (int i = 0; i < cols; i++)
         for (int j = 0; j < rows; j++)
            ret.m[j][i] = _self->m[i][j];

      return ret;
   }

   float &operator() (int row, int col)
   {
      return _self->m[col][row];
   }

   float operator() (int row, int col) const
   {
      return _self_const->m[col][row];
   }

   T &scale(float scalar)
   {
      for (int i = 0; i < cols; i++)
         for (int j = 0; j < rows; j++)
            _self->m[i][j] *= scalar;

      return *_self;
   }

   T &add(const T &rhs)
   {
      for (int i = 0; i < cols; i++)
         for (int j = 0; j < rows; j++)
            _self->m[i][j] += rhs.m[i][j];

      return *_self;
   }

   T &sub(const T &rhs)
   {
      for (int i = 0; i < cols; i++)
         for (int j = 0; j < rows; j++)
            _self->m[i][j] -= rhs.m[i][j];

      return *_self;
   }

   /*
   void rowSub(int row, int target, float multiplier)
   {
      for (int i = 0; i < cols; i++) {
         _self->m[i][target] -= _self->m[i][row] * multiplier;
      }
   }*/

};

template <typename T, int width, typename Vector_Type, typename B>
struct square_impl : public B {
   typedef DecompositionT<T, width> Decomposition;

   square_impl() {}
   square_impl(float val) : B(val) {}

   Decomposition LUPDecomposition() const
   {
      int N = width, kp;
      Decomposition ret;
      T temp(*_self_const);

      ret.valid = false;

      for (int i = 0; i < N; i++)
         ret.P[i] = i;

      for (int k = 0; k < N; k++) {
         float p = 0;
         for (int i = k; i < N; i++) {
            if (abs(temp(i,k)) > p) {
               p = abs(temp(i,k));
               kp = i;
            }
         }
         if (p == 0)
            return ret; // singular matrix
         swap(ret.P[k], ret.P[kp]);
         for (int i = 0; i < N; i++) {
            swap(temp(k,i), temp(kp,i));
         }
         for (int i = k+1; i < N; i++) {
            temp(i,k) = temp(i,k)/temp(k,k);
            for (int j = k+1; j < N; j++) {
               temp(i,j) = temp(i,j) - temp(i,k)*temp(k,j);
            }
         }
      }

      for (int i = 0; i < N; i++) {
         for (int j = 0; j < N; j++) {
            if (i == j) { // diagonol
               ret.L(i,j) = 1;
               ret.U(i,j) = temp(i,j);
            } else if (i < j) { // row < col, upper
               ret.L(i,j) = 0;
               ret.U(i,j) = temp(i,j);
            } else { // row > col, lower
               ret.L(i,j) = temp(i,j);
               ret.U(i,j) = 0;
            }
         }
      }

      ret.valid = true;

      return ret;
   }

   Vector_Type solve(const Vector_Type &Bm) const
   {
      return solve(Bm, _self_const->LUPDecomposition());
   }

   static Vector_Type solve(const Vector_Type &Bm, const Decomposition &dcomp)
   {
      Vector_Type x(0), y(0);

      if (!dcomp.valid)
         return Vector_Type(0);

      for (int i = 0; i < width; i++) {
         float sum = 0;
         for (int j = 0; j < i; j++)
            sum += dcomp.L(i,j) * y[j];
         y[i] = Bm[dcomp.P[i]] - sum;
      }

      for (int i = width-1; i >= 0; i--) {
         float sum = 0;
         for (int j = i+1; j < width; j++)
            sum += dcomp.U(i,j) * x[j];
         x[i] = (y[i] - sum) / dcomp.U(i,i);
      }

      return x;
   }

   static T inverse(const Decomposition &dcomp)
   {
      T ret;

      if (!dcomp.valid)
         return T();

      for (int i = 0; i < width; i++) {
         Vector_Type Bm(0);
         Bm[i] = 1;
         Vector_Type X = solve(Bm, dcomp);
         for (int j = 0; j < width; j++)
            ret(j,i) = X[j];
      }

      return ret;
   }

   T inverse() const
   {
      return inverse(_self_const->LUPDecomposition());
   }

   static float determinant(const Decomposition &dcomp)
   {
      if (!dcomp.valid)
         return 0;

      float ret = 1;

      for (int i = 0; i < width; i++)
         ret *= dcomp.U(i,i);

      return ret * (width % 2 == 0 ? 1 : -1);
   }

   float determinant() const
   {
      return determinant(_self_const->LUPDecomposition());
   }

};

template <typename T, int rows, typename B>
struct vector_impl : public B {
   vector_impl() {}
   vector_impl(float val) : B(val) {}

   float length()
   {
      float ret = 0.0;

      for (int i = 0; i < rows; i++)
         ret += _self->v[i]*_self->v[i];

      return sqrt(ret);
   }

   T &normalize()
   {
      float len = _self->length();

      assert("normalizing vector of 0 length!" && len != 0);

      for (int i = 0; i < rows; i++)
         _self->v[i] /= len;

      return *_self;
   }

   float &operator[] (int i)
   {
      return _self->v[i];
   }

   float operator[] (int i) const
   {
      return _self_const->v[i];
   }

   float dot(const T &rhs) const
   {
      float ret = 0;

      for (int i = 0; i < rows; i++) {
         ret += _self_const->v[i] * rhs.v[i];
      }

      return ret;
   }
};

// ***************************
//      Class Declarations
// ***************************

#define _matrix_base(rows, cols) matrix_impl<matrix<rows, cols>, rows, cols, matrix<cols, rows> >
#define _vector_base(rows) vector_impl<matrix<rows,1>, rows, _matrix_base(rows,1) >
#define _square_base(width) square_impl<matrix<width, width>, width, matrix<width, 1>, _matrix_base(width,width)>


template <int rows, int cols>
struct matrix : public _matrix_base(rows,cols) {
   float m[cols][rows];
   matrix() {}
   matrix(float val) : _matrix_base(rows,cols)(val) {}
};

template<int width>
struct matrix<width, width> : public _square_base(width) {
   float m[width][width];
   matrix() {}
   matrix(float val) : _square_base(width)(val) {}
};

template<int rows>
struct matrix<rows, 1> : public _vector_base(rows) {
   matrix() {}
   matrix(float val) : _vector_base(rows)(val) {}
   union {
      float m[1][rows];
      float v[rows];
   };
};

template <>
struct matrix<1,1> : public _vector_base(1) {
   matrix() {}
   matrix(float val) : _vector_base(1)(val) {}
   operator float() { return m[0][0]; }
   union {
      float m[1][1];
      float v[1];
   };
};

template <>
struct matrix<2,1> : public _vector_base(2) {
   union {
      float m[1][2];
      float v[2];
      struct { float x, y; };
   };
   matrix() {}
   matrix(float val) : _vector_base(2)(val) {}
   matrix(float x, float y) {
      this->x = x;
      this->y = y;
   }

   inline matrix<3,1> vec3(float z = 0) const;
   inline matrix<3,1> vec3xz(float y = 0) const;
};

template <>
struct matrix<3,1> : public _vector_base(3) {
   union {
      float m[1][3];
      float v[3];
      struct { float x, y, z; };
      //struct { float r, g, b; }; bleh
   };
   matrix() {}
   matrix(float val) : _vector_base(3)(val) {}
   matrix(float x, float y, float z) {
      this->x = x;
      this->y = y;
      this->z = z;
   }

   matrix<2,1> xy() const { return matrix<2,1>(x, y); }
   matrix<2,1> xz() const { return matrix<2,1>(x, z); }
   inline matrix<4,1> vec4(float w = 0) const;
};

template <>
struct matrix<4,1> : public _vector_base(4) {
   union {
      float m[1][4];
      float v[4];
      struct { float x, y, z, w; };
   };
   matrix() {}
   matrix(float val) : _vector_base(4)(val) {}
   matrix(float x, float y, float z, float w) {
      this->x = x;
      this->y = y;
      this->z = z;
      this->w = w;
   }

   matrix<3,1> xyz() const { return matrix<3,1>(x, y, z); }
   matrix<3,1> vec3() const { return xyz(); }
};

template<int rows>
struct vec : public matrix<rows, 1> {
   vec() {}
   vec(float val) : matrix<rows, 1>(val) {}
   vec(const matrix<rows, 1> &src) : matrix<rows, 1>(src) {}
   operator matrix<rows, 1> () { 
      return matrix<rows, 1>(*static_cast< matrix<rows, 1>* >(this));
   }
};

matrix<3,1> matrix<2,1>::vec3(float z) const
{
   return matrix<3,1>(x, y, z);
}

matrix<3,1> matrix<2,1>::vec3xz(float y) const
{
   return matrix<3,1>(x, y, this->y);
}

matrix<4,1> matrix<3,1>::vec4(float w) const
{
   return matrix<4,1>(x, y, z, w);
}


// ***************************
//         Aliases
// ***************************

typedef matrix<2,1> vec2;
typedef matrix<3,1> vec3;
typedef matrix<4,1> vec4;
typedef matrix<2,2> matrix2;
typedef matrix<3,3> matrix3;
typedef matrix<4,4> matrix4;


// ***************************
//     Matrix Operations
// ***************************


template <int M, int N>
inline matrix<M, N> scale(const matrix<M, N> &mat, float scalar)
{
   matrix<M, N> ret(mat);
   return ret.scale(scalar);
}

template <int M, int N>
inline matrix<M, N> add(const matrix<M, N> &lhs, const matrix<M, N> &rhs)
{
   matrix<M, N> ret(lhs);
   return ret.add(rhs);
}

template <int M, int N>
inline matrix<M, N> sub(const matrix<M, N> &lhs, const matrix<M, N> &rhs)
{
   matrix<M, N> ret(lhs);
   return ret.sub(rhs);
}

template <int M, int N, int O, int P>
inline matrix<O, P> resize(const matrix<M, N> &lhs)
{
   matrix<O, P> ret();

   for (int i = 0; i < std::min(M, O); i++)
      for (int j = 0; j < std::min(N, P); j++)
         ret(i,j) = lhs(i,j);

   return ret;
}


template <int M, int N>
inline matrix<M, N> lin(const matrix<M, N> &lhs, const matrix<M, N> &rhs, float t)
{
   return add(scale(lhs, 1-t), scale(rhs, t));
}

template <int M, int N, int P>
inline matrix<M, P> operator* (const matrix<M, N> &lhs, const matrix<N, P> &rhs)
{
   matrix<M, P> ret(0);

   for (int m = 0; m < M; m++) {
      for (int p = 0; p < P; p++) {
         for (int n = 0; n < N; n++) {
            ret(m,p) += lhs(m,n) * rhs(n,p);
         }
      }
   }

   return ret;
}

inline vec3 operator* (const matrix4 &lhs, const vec3 &rhs)
{
   vec4 v = rhs.vec4(1);

   vec4 ret = lhs * v;

   return ret.xyz().scale(1/ret.w);
}

template <int M, int N>
inline matrix<M, N> operator* (const matrix<M, N> &lhs, float scalar)
{
   return scale(lhs, scalar);
}

template <int M, int N>
inline matrix<M, N> operator/ (const matrix<M, N> &lhs, float scalar)
{
   return scale(lhs, 1/scalar);
}

template <int M, int N>
inline matrix<M, N> operator* (float scalar, const matrix<M, N> &rhs)
{
   return scale(rhs, scalar);
}

template <int M, int N>
inline matrix<M, N> operator+ (const matrix<M, N> &lhs, const matrix<M, N> &rhs)
{
   matrix<M, N> ret(0);

   for (int m = 0; m < M; m++)
      for (int n = 0; n < N; n++)
         ret(m,n) = lhs(m,n) + rhs(m,n);

   return ret;
}

template <int M, int N>
inline matrix<M, N> operator- (const matrix<M, N> &lhs, const matrix<M, N> &rhs)
{
   matrix<M, N> ret(0);

   for (int m = 0; m < M; m++)
      for (int n = 0; n < N; n++)
         ret(m,n) = lhs(m,n) - rhs(m,n);

   return ret;
}



/***************************
    Matrix Generators
****************************/

template <int N>
inline matrix<N, N> identity()
{
   matrix<N, N> ret(0);

   for (int i = 0; i < N; i++)
      ret(i,i) = 1;

   return ret;
}

template <int N>
inline matrix<N, N> makeRotationMatrix(vec3 axis, float angle)
{
   matrix<N, N> ret = identity<N>();

   axis.normalize();
   float c = cos(angle);
   float s = sin(angle);
   float x = axis.x;
   float y = axis.y;
   float z = axis.z;
   float t = 1 - c;

   ret(0,0) = t*x*x + c;
   ret(0,1) = t*x*y - s*z;
   ret(0,2) = t*x*z + s*y;

   ret(1,0) = t*x*y + s*z;
   ret(1,1) = t*y*y + c;
   ret(1,2) = t*y*z - s*x;

   ret(2,0) = t*x*z - s*y;
   ret(2,1) = t*y*z + s*x;
   ret(2,2) = t*z*z + c;

   return ret;
}

inline matrix3 rotationMatrix3(const vec3 &axis, float angle)
{
   return makeRotationMatrix<3>(axis, angle);
}

inline matrix4 rotationMatrix4(const vec3 &axis, float angle)
{
   return makeRotationMatrix<4>(axis, angle);
}

inline matrix4 translateMatrix4(float x, float y, float z)
{
   matrix4 ret = identity<4>();

   ret(0,3) = x;
   ret(1,3) = y;
   ret(2,3) = z;

   return ret;
}

inline matrix4 translateMatrix4(const vec3 &trans)
{
   return translateMatrix4(trans.x, trans.y, trans.z);
}

template <int N>
inline matrix<N, N> makeScaleMatrix(float x, float y, float z)
{
   matrix<N, N> ret = identity<N>();

   ret(0,0) = x;
   ret(1,1) = y;
   ret(2,2) = z;

   return ret;
}

inline matrix3 scaleMatrix3(const vec3 &scale)
{
   return makeScaleMatrix<3>(scale.x, scale.y, scale.z);
}

inline matrix3 scaleMatrix3(float x, float y, float z)
{
   return makeScaleMatrix<3>(x, y, z);
}

inline matrix4 scaleMatrix4(const vec3 &scale)
{
   return makeScaleMatrix<4>(scale.x, scale.y, scale.z);
}

inline matrix4 scaleMatrix4(float x, float y, float z)
{
   return makeScaleMatrix<4>(x, y, z);
}

/***************************
    Vector Free Functions
****************************/

template <int M>
inline matrix<M, 1> normalize(const matrix<M, 1> &lhs)
{
   matrix<M, 1> ret(lhs);
   return ret.normalize();
}

// Currently, cross product for only 3 dimensional vectors
inline matrix<3, 1> cross(const matrix<3, 1> &lhs, const matrix<3, 1> &rhs)
{
   return matrix<3, 1>(lhs.y*rhs.z - lhs.z*rhs.y,
                       lhs.z*rhs.x - lhs.x*rhs.z,
                       lhs.x*rhs.y - lhs.y*rhs.x);
}

inline matrix<3, 1> ncross(const matrix<3, 1> &lhs, const matrix<3, 1> &rhs)
{
   return cross(lhs, rhs).normalize();
}

template <int M>
inline float dist(const matrix<M, 1> &lhs, const matrix<M, 1> &rhs)
{
   return sub(lhs, rhs).length();
}

template <int M>
inline matrix<M, 1> to(const matrix<M, 1> &lhs, const matrix<M, 1> &rhs)
{
   return sub(rhs, lhs);
}

} // end namespace

#endif
