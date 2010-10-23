#include "Camera.h"
#include "GLUtil.h"

using namespace mat;

void FpsCamera::init(const mat::vec3 &pos, const mat::vec3 &dir)
{
   this->pos = pos;
   this->dir = normalize(dir);
   up = vec3(0,1,0);
   maxAngle = 80.0;
}

void FpsCamera::moveForward(float dist)
{
   pos = pos + scale(dir, dist);
}

void FpsCamera::moveLeft(float dist)
{
   pos = pos + scale(ncross(up, dir), dist);
}

void FpsCamera::moveUp(float dist)
{
   pos = pos + scale(ncross(cross(up, dir), dir), dist);
}

void FpsCamera::rotateDown(float angle)
{
   float curAngle = -getPitch(false);
   angle = toRad(angle);

   if (curAngle + angle > toRad(maxAngle))
      angle = toRad(maxAngle) - curAngle;
   else if (curAngle + angle < -toRad(maxAngle))
      angle = -toRad(maxAngle) - curAngle;

   dir = rotationMatrix3(ncross(up, dir), angle) * dir;
   dir.normalize();
}

void FpsCamera::rotateLeft(float angle)
{
   dir = rotationMatrix3(up, toRad(angle)) * dir;
   dir.normalize();
}

void FpsCamera::doTransform()
{
   vec3 at = pos + dir;
   gluLookAt(pos.x, pos.y, pos.z,
             at.x,  at.y,  at.z,
             up.x,  up.y,  up.z);
}

float FpsCamera::getPitch(bool degrees)
{
   if (degrees)
      return toDeg(PI/2 - acos(dir.dot(up)));
   else
      return PI/2 - acos(dir.dot(up));
}