#ifndef _INPUTMANAGER_H_
#define _INPUTMANAGER_H_

#include "GLUtil.h"
#ifdef WIN32
#include <SDL_keysym.h>
#include <SDL_mouse.h>
#else
#include <SDL/SDL_keysym.h>
#include <SDL/SDL_mouse.h>
#endif
#include <vector>

#define IMP_NUMKEYS SDLK_LAST+8

#define IPM_LEFT_MOUSE ( SDL_BUTTON_LEFT + SDLK_LAST )
#define IPM_MIDDLE_MOUSE ( SDL_BUTTON_MIDDLE + SDLK_LAST )
#define IPM_RIGHT_MOUSE ( SDL_BUTTON_RIGHT + SDLK_LAST )
#define IPM_WHEEL_UP ( SDL_BUTTON_WHEELUP + SDLK_LAST )
#define IPM_WHEEL_DOWN ( SDL_BUTTON_WHEELDOWN + SDLK_LAST )
#define IPM_MOUSE4 ( SDL_BUTTON_X1 + SDLK_LAST )
#define IPM_MOUSE5 ( SDL_BUTTON_X2 + SDLK_LAST )

#define IPM_UP false;
#define IPM_DOWN true;

union SDL_Event;
struct VideoMode;

class InputManager {
public:
   //
   // Querey functions
   //

   // is the key being held down right now?
   bool keyDown(int k) { return keystates[k]; }
   // was the key pressed down THIS frame?
   bool keyPressed(int k) { return keystates[k] && changed[k]; }
   // was the key released THIS frame?
   bool keyReleased(int k) { return !keystates[k] && changed[k]; }
   // did the key change state THIS frame?
   bool keyChanged(int k) { return changed[k]; }
   int dx() { return move[0]; }
   int dy() { return move[1]; }
   int x() { return pos[0]; }
   int y() { return pos[1]; }

   //
   // Mouse state modifier functions
   //
   void captureMouse();
   void releaseMouse();
   void toggleCapture() { if (captureMode) releaseMouse(); else captureMouse(); }

   //
   // Update functions
   //
   void init(glUtilSettings *videoMode, bool capMouse);
   void nextFrame();
   void handleEvent(const SDL_Event &e);

private:
   bool captureMode;
   int pos[2], move[2];
   bool keystates[IMP_NUMKEYS];
   bool changed[IMP_NUMKEYS];
   glUtilSettings *vmode;
   std::vector<int> changedKeys;
   

};

#endif
