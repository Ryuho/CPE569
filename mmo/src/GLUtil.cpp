#include "GLUtil.h"
using namespace mat;

namespace {
   int nativeWidth, nativeHeight, bitsPerPixel, flags;
   int currentWidth, currentHeight;
   SDL_Surface *surface;
}

void glUtilQuitErr(const char *msg)
{
   fprintf(stderr, "%s -- %s\n", msg, SDL_GetError());
   SDL_Quit();
   exit(-1);
}

   int width, height;
   bool fullScreen;
   float fov, nearPlane, farPlane;
   mat::vec3 clearColor;
   glUtilErrFunc err;

glUtilSettings::glUtilSettings()
{
   width = 1024;
   height = 768;
   fullScreen = false;
   err = glUtilQuitErr;
}

void glUtilInit(glUtilSettings &settings)
{
   const SDL_VideoInfo* info;

   if (SDL_Init(SDL_INIT_VIDEO) < 0)
      settings.err("SDL_Init(SDL_INIT_VIDEO) failed.");
   
   if ( (info = SDL_GetVideoInfo()) == 0)
      settings.err("SDL_GetVideoInfo() failed.");

   nativeWidth = info->current_w;
   nativeHeight = info->current_h;
   bitsPerPixel = info->vfmt->BitsPerPixel;

   if (settings.fullScreen) {
      flags = SDL_OPENGL | SDL_FULLSCREEN;
      settings.width = nativeWidth;
      settings.height = nativeHeight;
   } else {
      flags = SDL_OPENGL;
   }

   surface = SDL_SetVideoMode(
      settings.width,
      settings.height,
      bitsPerPixel,
      flags);

   if (surface == 0)
      settings.err("SDL_SetVideoMode() failed.");
   
   SDL_WM_SetCaption("", "");
}

void glUtilReset()
{
   
}

void glUtilDrawOrigin()
{
   glPushMatrix();
   glLoadIdentity();
   glDisable(GL_LIGHTING);

   glBegin(GL_LINES);

   glColor3ub(255, 0, 0);
   glVertex3fv(vec3(0,0,0).v);
   glVertex3fv(vec3(1,0,0).v);

   glColor3ub(0, 255, 0);
   glVertex3fv(vec3(0,0,0).v);
   glVertex3fv(vec3(0,1,0).v);

   glColor3ub(0, 0, 255);
   glVertex3fv(vec3(0,0,0).v);
   glVertex3fv(vec3(0,0,1).v);

   glEnd();

   glPopMatrix();
}

int glUtilTypeSize(GLenum type)
{
   if (type == GL_BYTE ||
       type == GL_UNSIGNED_BYTE)
      return 1;
   if (type == GL_SHORT ||
       type == GL_UNSIGNED_SHORT)
      return 2;
   if (type == GL_INT ||
       type == GL_UNSIGNED_INT)
      return 4;
   printf("Asked for type: %X, don't have an entry for it\n", type);
   return -1;
}

void glUtilPrintError(const char *info)
{
   int error = glGetError();
   while (error != GL_NO_ERROR) {
      printf("%s: %s\n", info, gluErrorString(error));
      error = glGetError();
   }
}

void glUtilPrintShaderVariables(int program)
{
   int uniforms, strl, usize;
   GLenum en;
   char buffer[260];

   glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &uniforms);
   printf("active attributes: %d\n", uniforms);
   for (int i = 0; i < uniforms; i++) {
      glGetActiveAttrib(program, i, 256, &strl, &usize, &en, buffer);
      printf("  attribute: %s\n", buffer);
   }

   glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniforms);
   printf("active uniforms: %d\n", uniforms);

   for (int i = 0; i < uniforms; i++) {
      glGetActiveUniform(program, i, 256, &strl, &usize, &en, buffer);
      printf("  uniform: %s\n", buffer);
   }
}

void glUtilSetWireframe(bool on)
{
   if (on) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      glDisable(GL_LIGHTING);
   } else {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glEnable(GL_LIGHTING);
   }
}

bool glUtilQueryMemSize(int bytes)
{
   return false;
   // GL_TEXTURE_PROXY?
}