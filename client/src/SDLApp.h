#ifndef _SDL_APP_H_
#define _SDL_APP_H_

#include "GLUtil.h"
#include "InputManager.h"
#include "World.h"

using mat::vec3;

struct SDLApp {

   void init();
   void update();
   void draw();

   void handleEvents();
   void quit();

   glUtilSettings settings;
   InputManager inputMgr;

   int ticks, dt; // dt is an integer value, miliseconds that have passed
   float fdt; // fdt is a floating point value in seconds

   World world;
};

#endif