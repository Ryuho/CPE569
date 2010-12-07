#include "BotApp.h"
#include <ctime>
using namespace mat;

void BotApp::init(const char *host, int port)
{
   // Initalize connection before showing graphics
   srand((unsigned)time(0));
   world.init(host, port);
   alive = true;

   // Set up the window and other stuff
   ticks = currentTicks();
   dt = 0;
}

void BotApp::update()
{
   dt = currentTicks() - ticks;
   ticks += dt;
   fdt = dt / 1000.0f;

   world.update(ticks, fdt);
   alive = world.data.alive;
}

void BotApp::quit()
{
   exit(0);
}
