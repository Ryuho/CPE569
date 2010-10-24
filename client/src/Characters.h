#ifndef _CHARACTERS_H_
#define _CHARACTERS_H_

#include "GLUtil.h"
#include "Sprite.h"

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

struct Item {
   enum Type { Rupee };
   Item() : alive(false) {}
   
   void init(vec2 pos, Type type);
   void update(float fdt, Player &player);
   void draw();
   
   Type type;
   vec2 pos;
   bool alive;
};

struct vec2i {
   int x, y;
   vec2i() : x(0), y(0) {}
   vec2i(int x, int y) : x(x), y(y) {}
};

struct Animation {
   enum Type { LeftRight, Forward, Normal };
   //constant means the animation does not stop when not "moving"
   bool alwaysAnim;
   int animStart, speed;
   Type type;
   Sprite *sprite;

   Animation() : alwaysAnim(false), sprite(0), type(Animation::Normal), 
      animStart(0), speed(100) {};
   void init(Sprite *sprite, Type type, bool alwaysAnim);
   void draw(vec2 dir, bool moving);
   static int dirFace(vec2 dir); //(up=0, right=1, down=2, left=3, error)
   static int dirFaceLR(vec2 dir); //(right=1, left=3, error)

   //FIRST vec2i in vector is for the non-moving sprite, the others are for moving
   std::vector<vec2i> dirs[4]; //up, right, down, left
};

struct NPC {
   enum Type { Thief, Princess, Fairy, Skeleton, Cyclops, 
      Bat, Bird, Squirrel, Chicken, Vulture, Bush, Cactus, 
      BigFairy, Wizard, Ganon, Goblin, MaxNPC };
   NPC() : alive(false), anim(0) {}
   
   void init(vec2 pos, Type type);
   void update(float fdt, Player &player);
   void draw();
   
   Type type;
   vec2 pos, dir;
   bool alive, moving;
   Animation *anim;
};

#endif
