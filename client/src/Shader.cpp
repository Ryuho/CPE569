#include "Shader.h"
#include "GLUtil.h"
#include <string>
#include <fstream>
#include <vector>

using namespace std;

struct ShaderBase {
   ShaderBase(const char *fn, GLenum type);
   void destroy();

   unsigned shader;
   GLenum type;
   bool valid;
   string fn;
};

struct ProgramData {
   ProgramData(VertexShader vs, FragmentShader fs);
   void enable();
   void disable();
   int getAttrib(const char *name);
   int getUniform(const char *name);
   int getParam(const char *name, bool uniform);
   void destroy();

   VertexShader vs;
   FragmentShader fs;
   unsigned program;
   bool valid, active, immediate;
};

ShaderBase::ShaderBase(const char *fn, GLenum type)
   : valid(false), type(type), fn(fn)
{
   string text;
   ifstream file(fn);
   int shaderValue;
   const char *textPtr;

   if (!file.good()) {
      printf("Could not open shader file: %s\n", fn);
      return;
   }
   
   getline(file, text, '\0');

   shader = glCreateShader(type);
   shaderValue = text.size();
   textPtr = text.c_str();
   glShaderSource(shader, 1, &textPtr, &shaderValue);
   glCompileShader(shader);

   glGetShaderiv(shader, GL_COMPILE_STATUS, &shaderValue);
   if (!shaderValue) {
      printf("Could not compile %s:\n", fn);
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &shaderValue);
      char *log = new char[shaderValue];
      glGetShaderInfoLog(shader, shaderValue, &shaderValue, log);
      printf("%s\n", log);
      delete[] log;
      glDeleteShader(shader);
      return;
   }

   glUtilPrintError("Compile Shader");
   valid = true;
}

void ShaderBase::destroy()
{
   if (valid) {
      glDeleteShader(shader);
      valid = false;
   }
}

ProgramData::ProgramData(VertexShader vs, FragmentShader fs)
   : valid(false), active(false), immediate(false), vs(vs), fs(fs)
{
   int programValue;
   if (!vs.data->valid || !fs.data->valid) {
      printf("One or more invalid shaders passed to program\n");
      return;
   }

   program = glCreateProgram();
   glAttachShader(program, vs.data->shader);
   glAttachShader(program, fs.data->shader);
   glLinkProgram(program);

   glGetProgramiv(program, GL_LINK_STATUS, &programValue);
   if (!programValue) {
      printf("Could not link shader program: %s and %s\n",
         vs.data->fn.c_str(), fs.data->fn.c_str());
      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &programValue);
      char *log = new char[programValue];
      glGetProgramInfoLog(program, programValue, &programValue, log);
      printf("%s\n", log);
      delete[] log;
      glDeleteProgram(program);
      return;
   }

   //glUtilPrintShaderVariables(program);
   glUtilPrintError("Shader Program");
   valid = true;
}

void ProgramData::enable()
{
   glUseProgram(program);
   active = true;
}

void ProgramData::disable()
{
   glUseProgram(0);
   active = false;
}

int ProgramData::getAttrib(const char *name)
{
   return getParam(name, false);
}

int ProgramData::getUniform(const char *name)
{
   return getParam(name, true);
}

int ProgramData::getParam(const char *name, bool uniform)
{
   int loc = uniform ? 
      glGetUniformLocation(program, name):
      glGetAttribLocation(program, name);

   if (loc < 0) {
      printf("Invalid %s name: %s\n", uniform ? "uniform" : "attrib", name);
      glUtilPrintError("Adding shader uniform/attribute");
      return -1;
   }

   return loc;
}

void ProgramData::destroy()
{
   printf("destroying\n");
   if (valid) {
      glDetachShader(program, vs.data->shader);
      glDetachShader(program, fs.data->shader);
      glDeleteProgram(program);
      valid = false;
   }
}

VertexShader::VertexShader(const char *fn) : data(new ShaderBase(fn, GL_VERTEX_SHADER)) {}
void VertexShader::destroy() { data->destroy(); }

FragmentShader::FragmentShader(const char *fn) : data(new ShaderBase(fn, GL_FRAGMENT_SHADER)) {}
void FragmentShader::destroy() { data->destroy(); }

ShaderProgram::ShaderProgram(VertexShader vs, FragmentShader fs)
   : data(new ProgramData(vs, fs)) {}
void ShaderProgram::enable() { data->enable(); }
void ShaderProgram::disable() { data->disable(); }
int ShaderProgram::getAttrib(const char *name) { return data->getAttrib(name); }
int ShaderProgram::getUniform(const char *name) { return data->getUniform(name); }
void ShaderProgram::destroy() { data->destroy(); }
void ShaderProgram::setImmediate(bool value) { data->immediate = value; }