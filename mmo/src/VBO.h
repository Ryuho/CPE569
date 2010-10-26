#ifndef _VBO_H_
#define _VBO_H_

#include "TriangleSet.h"
#include "GLUtil.h"
#include <boost/shared_ptr.hpp>

struct VBOData;

struct VBO {
   VBO() {}
   VBO(const TriangleSet &set);
   VBO(void *vertData, int size, int vcount, void *indexData, GLenum type,
       int icount, GLenum mode = GL_TRIANGLES, GLenum usage = GL_STATIC_DRAW);

   void draw();
   void destroy();
   void vertPos(int offset);
   void normalPos(int offset);
   void attribPos(int attrib, int count, GLenum type, int offset, bool normalized = false);

   boost::shared_ptr<VBOData> data;
};

#endif