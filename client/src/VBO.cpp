#include "VBO.h"
#include <cstdio>

namespace {

struct VertData {
   vec3 v;
   vec3 n;
};

struct AttribData {
   AttribData(int location, int count, GLenum type, int offset, bool norm)
      : location(location), count(count), type(type), offset(offset), norm(norm) {}
   int location, count, offset;
   GLenum type;
   bool norm;
};

}

struct VBOData {
   VBOData() : valid(false) {}
   VBOData(const TriangleSet &set);
   void initalize(void *vertData, int size, int vcount, 
      void *indexData, GLenum type, int icount, GLenum mode, GLenum usage);
   void draw();
   void destroy();
   void vertPos(int offset);
   void normalPos(int offset);
   void attribPos(int attrib, int count, GLenum type, int offset, bool normalized);

   bool valid;
   unsigned vbo, ibo;
   int vcount, icount;
   int stride, voffset, noffset;
   vector<AttribData> attribs;
};

//extern int nloc, vloc;

VBOData::VBOData(const TriangleSet &set) : valid(false)
{
   TriangleSet ts = set.cleanUp();
   VertData *vd = new VertData[set.verts.size()];
   Face *id = new Face[set.faces.size()];

   for (unsigned i = 0; i < ts.verts.size(); i++) {
      vd[i].v = ts.verts[i];
      vd[i].n = vec3(0, 0, 0);
   }

   for (unsigned i = 0; i < ts.faces.size(); i++) {
      id[i] = ts.faces[i];
      vec3 norm = ncross(to(vd[id[i].v1].v, vd[id[i].v2].v), 
                         to(vd[id[i].v1].v, vd[id[i].v3].v));
      vd[id[i].v1].n = vd[id[i].v1].n + norm;
      vd[id[i].v2].n = vd[id[i].v2].n + norm;
      vd[id[i].v3].n = vd[id[i].v3].n + norm;
   }
   
   for (unsigned i = 0; i < ts.verts.size(); i++) {
      vd[i].n.normalize();
   }

   initalize(vd, sizeof(VertData), ts.verts.size(),
      id, GL_INT, ts.faces.size()*3, GL_TRIANGLES, GL_STATIC_DRAW);

   vertPos(offsetof(VertData, v));
   normalPos(offsetof(VertData, n));
   //attribPos(vloc, 3, GL_FLOAT, offsetof(VertData, v), false);
   //attribPos(nloc, 3, GL_FLOAT, offsetof(VertData, n), false);

   delete[] vd;
   delete[] id;
}

void VBOData::initalize(void *vertData, int size, int vcount, 
 void *indexData, GLenum type, int icount, GLenum mode, GLenum usage)
{
   this->vcount = vcount;
   this->icount = icount;
   stride = size;
   voffset = -1;
   noffset = -1;

   //printf("size: %d, vcount: %d, icount: %d\n", size, vcount, icount);
   //printf("vd: %p, id: %p\n", vertData, indexData);
   
   glGenBuffers(1, &vbo);
   glBindBuffer(GL_ARRAY_BUFFER, vbo);
   glBufferData(GL_ARRAY_BUFFER, size*vcount, vertData, usage);

   glGenBuffers(1, &ibo);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, glUtilTypeSize(type)*icount,
      indexData, usage);

   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

   valid = true;
}

void VBOData::draw()
{
   if (!valid) {
      printf("Trying to draw invalid VBO\n");
      return;
   }

   glBindBuffer(GL_ARRAY_BUFFER, vbo);

   if (voffset >= 0) {
      glEnableClientState(GL_VERTEX_ARRAY);
      glVertexPointer(3, GL_FLOAT, stride, (void*)voffset);
   }
   if (noffset >= 0) {
      glEnableClientState(GL_NORMAL_ARRAY);
      glNormalPointer(GL_FLOAT, stride, (void*)noffset);
   }
   for (unsigned i = 0; i < attribs.size(); i++) {
      glEnableVertexAttribArray(attribs[i].location);
      glVertexAttribPointer(attribs[i].location,
         attribs[i].count, attribs[i].type, attribs[i].norm,
         stride, (void*)attribs[i].offset);
   }

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
   glDrawElements(GL_TRIANGLES, icount, GL_UNSIGNED_INT, 0);

   if (voffset >= 0)
      glDisableClientState(GL_VERTEX_ARRAY);
   if (noffset >= 0)
      glDisableClientState(GL_NORMAL_ARRAY);
   for (unsigned i = 0; i < attribs.size(); i++)
      glDisableVertexAttribArray(attribs[i].location);

   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void VBOData::vertPos(int offset)
{
   voffset = offset;
}

void VBOData::normalPos(int offset)
{
   noffset = offset;
}

void VBOData::attribPos(int attrib, int count, GLenum type, int offset, bool normalized)
{
   attribs.push_back(AttribData(attrib, count, type, offset, normalized));
}

void VBOData::destroy()
{
   if (valid) {
      glDeleteBuffers(1, &vbo);
      glDeleteBuffers(1, &ibo);
      valid = false;
   }
}

VBO::VBO(const TriangleSet &set) : data(new VBOData(set)) {}
VBO::VBO(void *vertData, int size, int vcount, void *indexData,
         GLenum type, int icount, GLenum mode, GLenum usage)
         : data(new VBOData) {
   data->initalize(vertData, size, vcount, indexData, type, icount, mode, usage);
}
void VBO::draw() { data->draw(); }
void VBO::destroy() { data->destroy(); }
void VBO::vertPos(int offset) { data->vertPos(offset); }
void VBO::normalPos(int offset) { data->normalPos(offset); }
void VBO::attribPos(int attrib, int count, GLenum type, int offset, bool normalized)
{
   data->attribPos(attrib, count, type, offset, normalized);
}

