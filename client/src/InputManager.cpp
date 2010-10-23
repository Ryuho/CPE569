#include "InputManager.h"
#include "string.h"

using std::vector;

/***********************************
 *        Update Functions         *
 ***********************************/

void InputManager::init(glUtilSettings *videoMode, bool capMouse)
{
   vmode = videoMode;
   captureMode = capMouse;
   memset(keystates, 0, IMP_NUMKEYS*sizeof(bool));
   memset(changed, 0, IMP_NUMKEYS*sizeof(bool));
   if (capMouse)
      captureMouse();
   else
      releaseMouse();
   nextFrame();
}

void InputManager::nextFrame()
{
   int x, y;

   SDL_GetMouseState(&x, &y);
   move[0] = x - pos[0];
   move[1] = y - pos[1];
   if (captureMode) {
      SDL_WarpMouse(vmode->width/2, vmode->height/2);
      pos[0] = vmode->width/2;
      pos[1] = vmode->height/2;
   } else {
      pos[0] = x;
      pos[1] = y;
   }

   // unset all the changed keys.
   for(vector<int>::iterator itr = changedKeys.begin(); itr != changedKeys.end(); ++itr)
      changed[*itr] = false;
   changedKeys.clear();
}

void InputManager::captureMouse()
{
   captureMode = true;
   SDL_WarpMouse(vmode->width/2, vmode->height/2);
   SDL_ShowCursor(SDL_DISABLE);
   SDL_WM_GrabInput(SDL_GRAB_ON);
   pos[0] = vmode->width/2;
   pos[1] = vmode->height/2;
   move[0] = 0;
   move[1] = 0;
}

void InputManager::releaseMouse()
{
   int x, y;

   captureMode = false;
   SDL_ShowCursor(SDL_ENABLE);
   SDL_WM_GrabInput(SDL_GRAB_OFF);
   SDL_GetMouseState(&x, &y);
   pos[0] = x;
   pos[1] = y;
   move[0] = 0;
   move[1] = 0;
}

/***********************************
 *        Event Handler            *
 ***********************************/

void InputManager::handleEvent(const SDL_Event &e) 
{
   switch(e.type) {
      case SDL_KEYDOWN:
         keystates[e.key.keysym.sym] = IPM_DOWN;
         changed[e.key.keysym.sym] = true;
         changedKeys.push_back(e.key.keysym.sym);
         break;
      case SDL_KEYUP:
         keystates[e.key.keysym.sym] = IPM_UP;
         changed[e.key.keysym.sym] = true;
         changedKeys.push_back(e.key.keysym.sym);
         break;
      case SDL_MOUSEBUTTONDOWN:
         keystates[e.button.button+SDLK_LAST] = IPM_DOWN;
         changed[e.button.button+SDLK_LAST] = true;
         changedKeys.push_back(e.button.button+SDLK_LAST);
         break;
      case SDL_MOUSEBUTTONUP:
         keystates[e.button.button+SDLK_LAST] = IPM_UP;
         changed[e.button.button+SDLK_LAST] = true;
         changedKeys.push_back(e.button.button+SDLK_LAST);
         break;
   }
}
