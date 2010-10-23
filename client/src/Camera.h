#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "matrix.h"
using mat::vec3;

struct FpsCamera
{
   FpsCamera() {}

   void init(const vec3 &pos = vec3(0,0,0), const vec3 &dir = vec3(0,0,1));
   void moveForward(float dist);
   void moveLeft(float dist);
   void moveUp(float dist);
   void rotateDown(float angle);
   void rotateLeft(float angle);

   float getPitch(bool degrees = true);

   void doTransform();

   vec3 pos, dir, up;
   float maxAngle;
};

#endif