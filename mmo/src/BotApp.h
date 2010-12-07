#ifndef _BOT_APP_H_
#define _BOT_APP_H_

#include <exception>
#include "matrix.h"
#include "BotWorld.h"

using mat::vec3;

struct BotApp {

   void init(const char *host, int port);
   void update();

   void quit();

   int ticks, dt; // dt is an integer value, miliseconds that have passed
   float fdt; // fdt is a floating point value in seconds

   BotWorld world;
   bool alive;
};

#endif //_BOT_APP_H_
