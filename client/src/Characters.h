#ifndef _CHARACTERS_H_
#define _CHARACTERS_H_

#include "GLUtil.h"

void initCharacterResources();

struct Player {
   Player() {}
   void update(vec2 pos, vec2 dir, bool moving);
   void draw();

   vec2 pos, dir;
   bool moving;
   int animStart;
};

struct Missile {
   enum Type { Arrow };
   Missile() : alive(false) {}

   void init(vec2 pos, vec2 dir, Type type);
   void update(float fdt);
   void draw();

   Type type;
   vec2 pos, dir, start;
   bool alive;
};

#endif