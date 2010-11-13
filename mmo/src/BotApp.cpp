#include "BotApp.h"
#include <ctime>
using namespace mat;

void BotApp::init(const char *host, int port)
{
   // Initalize connection before showing graphics
   srand((unsigned)time(0));
   world.init(host, port);

   // Set up the window and other stuff
   ticks = SDL_GetTicks();
   dt = 1;
}

void BotApp::update()
{
   dt = SDL_GetTicks() - ticks;
   ticks += dt;
   fdt = dt / 1000.0f;

   handleEvents();

   world.update(ticks, fdt);
}

void BotApp::handleEvents()
{
   SDL_Event ev;

   SDL_PumpEvents();
   while(SDL_PollEvent(&ev)) {
      if((ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE)
            || ev.type == SDL_QUIT) {
         quit();
      }
   }
}

void BotApp::quit()
{
   SDL_Quit();
   exit(0);
}
