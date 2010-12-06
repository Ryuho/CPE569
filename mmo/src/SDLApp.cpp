#include "SDLApp.h"
#include "FreeType.h"
#include <ctime>
using namespace mat;

void SDLApp::init(const char *host, int port)
{
   // Initalize connection before showing graphics
   srand(time(0));
   world.init(host,port);

   // Set up the window and other stuff
   settings.width = 1024;
   settings.height = 768;
   settings.fullScreen = false;
   ticks = SDL_GetTicks();
   dt = 1;
   glUtilInit(settings);

   glewInit();
   //if ( ! glewIsSupported("GL_VERSION_2_1") )
   //   settings.err("openGL version 2.1 not supported.");

   // set gouraud shading
   glShadeModel(GL_SMOOTH);

   // set culling options
   glCullFace(GL_BACK);
   glFrontFace(GL_CCW);
   glEnable(GL_CULL_FACE);
   glEnable(GL_DEPTH_TEST);

   // background color
   glClearColor(0.45, 0.60, 0.92, 0);
   glViewport(0, 0, settings.width, settings.height);

   // initalize components
   inputMgr.init(&settings, false);
   world.graphicsInit(settings.width, settings.height);
}

void SDLApp::update()
{
   dt = SDL_GetTicks() - ticks;
   ticks += dt;
   fdt = dt / 1000.0;

   inputMgr.nextFrame();
   handleEvents();

   if (inputMgr.keyDown(SDLK_ESCAPE))
      quit();

   if (inputMgr.keyDown(IPM_LEFT_MOUSE)) {
      world.shootArrow(vec2(inputMgr.x(), settings.height-inputMgr.y()));
   }
   
   if (inputMgr.keyPressed(SDLK_f)) {
      world.doSpecial();
   }
   if(inputMgr.keyChanged(IPM_RIGHT_MOUSE)) {
      world.rightClick(vec2(inputMgr.x(), settings.height-inputMgr.y()));
   }

   if (inputMgr.keyPressed(SDLK_e)) {
      world.spawnItem();
   }
   if (inputMgr.keyPressed(SDLK_r)) {
      world.hurtMe();
   }
   if (inputMgr.keyPressed(SDLK_p)) {
      world.togglePvp();
   }

   vec2 dir(0.0, 0.0);
   if (inputMgr.keyDown(SDLK_w))
      dir = dir + vec2(0.0, 1.0);
   if (inputMgr.keyDown(SDLK_s))
      dir = dir + vec2(0.0, -1.0);
   if (inputMgr.keyDown(SDLK_a))
      dir = dir + vec2(-1.0, 0.0);
   if (inputMgr.keyDown(SDLK_d))
      dir = dir + vec2(1.0, 0.0);
   
   world.move(dir);

   world.update(ticks, fdt);
}

void SDLApp::draw()
{
   // Clear the color and depth buffers.
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   // Camera Transformations
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glTranslatef(-1.0, -1.0, 0.0);
   glScalef(2.0,2.0,1.0);

   // World Transform
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glScalef(1.0/settings.width, 1.0/settings.height, 1/256.0);

   glColor3ub(255, 255, 255);

   world.draw();

   // End drawing
   glUtilPrintError("Swap Buffers");
   SDL_GL_SwapBuffers();
}

void SDLApp::handleEvents()
{
   SDL_Event ev;

   SDL_PumpEvents();
   while(SDL_PollEvent(&ev)) {
      inputMgr.handleEvent(ev);
      if (ev.type == SDL_QUIT)
         quit();
   }
}

void SDLApp::quit()
{
   SDL_Quit();
   exit(0);
}
