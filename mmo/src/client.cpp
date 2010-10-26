#include "SDLApp.h"


int main (int argc, char *argv[])
{
   SDLApp app;

   app.init();

   while (1) {
      app.update();
      app.draw();
   }

   return 0;
}