#ifndef _GL_UTIL_H_
#define _GL_UTIL_H_

#ifdef WIN32
#include <windows.h>
#include <SDL.h>
#else
#include <SDL/SDL.h>
#endif
#include <GL/glew.h>
#include <GL/glut.h>

#include <exception>
#include "matrix.h"
using mat::vec3;

const float PI = 3.14159265359; 
inline float toRad(float angle)
{
   return angle*PI/180.0;
}

inline float toDeg(float angle)
{
   return angle/PI*180.0;
}

typedef void (*glUtilErrFunc)(const char *msg);

struct glUtilSettings {
   glUtilSettings();

   int width, height;
   bool fullScreen;
   vec3 clearColor;
   glUtilErrFunc err;
};

void glUtilInit(glUtilSettings &settings);
void glUtilReset();
void glUtilDrawOrigin();
int glUtilTypeSize(GLenum type);
void glUtilPrintError(const char *info = "OPENGL ERROR");
void glUtilPrintShaderVariables(int program);
void glUtilSetWireframe(bool on);

#endif
