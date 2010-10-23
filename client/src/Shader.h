#ifndef _SHADER_H_
#define _SHADER_H_

#include <boost/shared_ptr.hpp>

struct ShaderBase;
struct ProgramData;

struct VertexShader {
   VertexShader(const char *fn);
   void destroy();
   boost::shared_ptr<ShaderBase> data;
};

struct FragmentShader {
   FragmentShader(const char *fn);
   void destroy();
   boost::shared_ptr<ShaderBase> data;
};

struct ShaderProgram {
   ShaderProgram() {}
   ShaderProgram(VertexShader vs, FragmentShader fs);
   void enable();
   void disable();
   void destroy();
   int getAttrib(const char *name);
   int getUniform(const char *name);
   void setImmediate(bool value = true);
   boost::shared_ptr<ProgramData> data;
};

#endif