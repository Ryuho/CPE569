#ifndef _GEOMETRY_H_
#define _GEOMETRY_H_

#include "matrix.h"

namespace geom {

   using namespace mat;

   struct Circle;
   struct Square;
   struct Plane;

   //May want to know where they collided?
   struct Geometry {
      virtual bool collision(Geometry *other) =0;

      virtual bool collision(Circle *other) = 0;
      virtual bool collision(Square *other) = 0;
      virtual bool collision(Plane *other) = 0;
   };

   struct Circle : Geometry {
      Circle(float radius=0, vec2 pos = vec2()) : radius(radius), pos(pos) {}
      void init(vec2 pos, float radius);
      bool collision(Geometry *other);
      bool collision(Circle *other);
      bool collision(Square *other);
      bool collision(Plane *other);

      float radius;
      vec2 pos;
   };

   struct Square : Geometry {
      Square(vec2 bottomLeft, vec2 size) : bottomLeft(bottomLeft), size(size) {}
      void init(vec2 bottomleft, vec2 size);
      bool collision(Geometry *other);
      bool collision(Circle *other);
      bool collision(Square *other);
      bool collision(Plane *other);

      vec2 bottomLeft, size;
   };

   struct Plane : Geometry {
      Plane(vec2 pos = vec2(0,0), float slope=0) : pos(pos), slope(slope) {}
      void init(vec2 pos, float slope);
      bool collision(Geometry *other);
      bool collision(Circle *other);
      bool collision(Square *other);
      bool collision(Plane *other);

      vec2 pos;
      float slope; //2d is Y = mX+b
   };
}

#endif //_GEOMETRY_H_