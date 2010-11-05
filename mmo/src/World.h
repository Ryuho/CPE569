#ifndef _WORLD_H_
#define _WORLD_H_

#include "Characters.h"
#include <boost/shared_ptr.hpp>

struct WorldData;

struct World {
   World() {}

   void init();
   void graphicsInit(int width, int height);
   void update(int ticks, float dt);
   void draw();

   void move(vec2 dir);
   void shootArrow(mat::vec2 dir);
   void doSpecial();
   void spawnItem();
   void spawnNPC();
   void rightClick(vec2 mousePos);

protected:
   boost::shared_ptr<WorldData> data;
};

int getTicks();
float getDt();
Player &getPlayer(); // available only on client

#endif
