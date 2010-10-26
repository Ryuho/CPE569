#ifndef _SPRITE_H_
#define _SPRITE_H_

#include "Texture.h"

struct Sprite {
   Sprite() {}
   Sprite(Texture tex, int sWidth, int sHeight);

   void draw(int x, int y);

   Texture tex;
   int sWidth, sHeight;
};



#endif