#include "Geometry.h"

namespace geom {
using namespace mat;
using namespace std;

bool CircleCircle(const Circle *c1, const Circle *c2)
{
   return dist(c1->pos, c2->pos) < c1->radius + c2->radius;
}

bool CirclePlane(const Circle *c, const Plane *p)
{
   return abs(p->norm.dot(c->pos) + p->d) < c->radius + p->radius;
}

bool PlanePlane(const Plane *p1, const Plane *p2)
{
   if (abs(p1->norm.dot(p2->norm)) == 1.0) // parallel planes
      return abs(p1->d - p2->d) < p1->radius + p2->radius;
   else
      return true;
}

bool CirclePoint(const Circle *c, const Point *p) 
{
   return dist(c->pos, p->pos) < c->radius;
}

bool PointPoint(const Point *p1, const Point *p2)
{
   return mat::dist(p1->pos, p2->pos) == 0.0f;
}

bool PlanePoint(const Plane *plane, const Point *point)
{
   return abs(plane->norm.dot(point->pos) + plane->d) < plane->radius;
}

bool RectangleCircle(const Rectangle *rec, const Circle *c)
{
   vec2 test = c->pos; 

   if(test.x < rec->botLeft.x)
      test.x = rec->botLeft.x;
   else if(test.x > rec->botLeft.x + rec->w)
      test.x = rec->botLeft.x + rec->w;
   if(test.y < rec->botLeft.y)
      test.y = rec->botLeft.y;
   else if(test.y > rec->botLeft.y + rec->h)
      test.y = rec->botLeft.y + rec->h;

   return mat::dist(test, c->pos) < c->radius;
}

bool RectanglePlane(const Rectangle *rec, const Plane *pl)
{
   printf("RectanglePlane unimplemented\n");
   return false;
}

bool RectanglePoint(const Rectangle *rec, const Point *p)
{
   return p->pos.x >= rec->botLeft.x 
      && p->pos.y >= rec->botLeft.y 
      && p->pos.x <= rec->botLeft.x + rec->w 
      && p->pos.y <= rec->botLeft.x + rec->h;
}

bool RectangleRectangle(const Rectangle *rec1, const Rectangle *rec2)
{
   return rec2->botLeft.x + rec2->w >= rec1->botLeft.x //r2.right >= r1.left
      && rec2->botLeft.x <= rec1->botLeft.x + rec1->w //r2.left <= r1.right
      && rec2->botLeft.y + rec2->h >= rec1->botLeft.y //r2.top >= r1.bot
      && rec2->botLeft.y <= rec1->botLeft.y + rec1->h; //r2.bot <= r1.top
}


///////////// CIRCLE /////////////
//////////////////////////////////
bool Circle::collision(const GeometryBase *other) const
{
   return other->collision(this);
}

bool Circle::collision(const Circle *other) const
{
   return CircleCircle(this, other);
}

bool Circle::collision(const Plane *other) const
{
   return CirclePlane(this, other);
}

bool Circle::collision(const Point *other) const
{
   return CirclePoint(this, other);
}

bool Circle::collision(const Rectangle *other) const
{
   return RectangleCircle(other, this);
}

///////////// PLANE /////////////
/////////////////////////////////
bool Plane::collision(const GeometryBase *other) const
{
   return other->collision(this);
}

bool Plane::collision(const Circle *other) const
{
   return CirclePlane(other, this);
}

bool Plane::collision(const Plane *other) const
{
   return PlanePlane(this, other);
}

bool Plane::collision(const Point *other) const
{
   return PlanePoint(this, other);
}

bool Plane::collision(const Rectangle *other) const
{
   return RectanglePlane(other, this);
}

///////////// POINT /////////////
/////////////////////////////////
bool Point::collision(const GeometryBase *other) const
{
   return other->collision(this);
}

bool Point::collision(const Circle *other) const
{
   return CirclePoint(other, this);
}

bool Point::collision(const Plane *other) const
{
   return PlanePoint(other, this);
}

bool Point::collision(const Point *other) const
{
   return PointPoint(this, other);
}

bool Point::collision(const Rectangle *other) const
{
   return RectanglePoint(other, this);
}

///////////// RECTANGLE /////////////
/////////////////////////////////////
bool Rectangle::collision(const GeometryBase *other) const
{
   return other->collision(this);
}

bool Rectangle::collision(const Circle *other) const
{
   return RectangleCircle(this, other);
}

bool Rectangle::collision(const Plane *other) const
{
   return RectanglePlane(this, other);
}

bool Rectangle::collision(const Point *other) const
{
   return RectanglePoint(this, other);
}

bool Rectangle::collision(const Rectangle *other) const
{
   return RectangleRectangle(this, other);
}

///////////// GEOMETRYSET /////////////
///////////////////////////////////////
bool GeometrySet::collision(const GeometryBase *other) const
{
   for (unsigned i = 0; i < set.size(); i++)
      if (!set[i].ptr->collision(other))
         return false;
   return true;
}

bool GeometrySet::collision(const Circle *other) const
{
   for (unsigned i = 0; i < set.size(); i++)
      if (!set[i].ptr->collision(other))
         return false;
   return true;
}

bool GeometrySet::collision(const Plane *other) const
{
   for (unsigned i = 0; i < set.size(); i++)
      if (!set[i].ptr->collision(other))
         return false;
   return true;
}

bool GeometrySet::collision(const Point *other) const
{
   for (unsigned i = 0; i < set.size(); i++)
      if (!set[i].ptr->collision(other))
         return false;
   return true;
}

bool GeometrySet::collision(const Rectangle *other) const
{
   for (unsigned i = 0; i < set.size(); i++)
      if (!set[i].ptr->collision(other))
         return false;
   return true;
}

} // end geom namespace
