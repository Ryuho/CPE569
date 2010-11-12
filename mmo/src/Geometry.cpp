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


} // end geom namespace
