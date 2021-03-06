#ifndef _SPRITE_H_
#define _SPRITE_H_

#include "Texture.h"
#include "matrix.h"
#include <vector>
#include "Constants.h"

using mat::vec2;
using namespace constants;

struct Sprite {
   Sprite() {}
   Sprite(Texture tex, int sWidth, int sHeight);

   void draw(int x, int y);

   Texture tex;
   int sWidth, sHeight;
};

struct vec2i {
   int x, y;
   vec2i() : x(0), y(0) {}
   vec2i(int x, int y) : x(x), y(y) {}
};

struct Animation {
   //constant means the animation does not stop when not "moving"
   bool alwaysAnim;
   int animStart, speed;
   int type;
   Sprite *sprite;

   Animation() : alwaysAnim(false), sprite(0), type(AnimType::Normal), 
      animStart(0), speed(100) {};
   void init(Sprite *sprite, int type, bool alwaysAnim);
   void draw();
   void draw(vec2 dir, bool moving);
   static int dirFace(vec2 dir); //(up=0, right=1, down=2, left=3, error)
   static int dirFaceLR(vec2 dir); //(right=1, left=3, error)

   //FIRST vec2i in vector is for the non-moving sprite, the others are for moving
   std::vector<vec2i> dirs[4]; //up, right, down, left
};


#endif