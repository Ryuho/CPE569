#ifndef _TRIANGLESET_H
#define _TRIANGLESET_H

#include "matrix.h"
#include <vector>
#include <map>
#include <set>
#include <cstdio>
#include <algorithm>

using namespace mat;
using namespace std;

template <typename T>
void vectorDelete(int index, vector<T> &from)
{
   from[index] = from.back();
   from.resize(from.size()-1);
}

template <typename T>
void vectorRemove(T value, vector<T> &from)
{
   typename vector<T>::iterator itr = find(from.begin(), from.end(), value);

   if (itr != from.end())
      vectorDelete(itr - from.begin(), from);
}

// Wrapper around a vec3 so that it can be placed in a tree
struct Vert {
   Vert(const vec3 &src) : vert(src) {}

   bool operator== (const Vert &rhs) const {
      return floatCompare(vert.x, rhs.vert.x)
          && floatCompare(vert.y, rhs.vert.y)
          && floatCompare(vert.z, rhs.vert.z);
   }

   bool operator< (const Vert &rhs) const {
      return !operator==(rhs) &&
             vert.x < rhs.vert.x || (vert.x == rhs.vert.x &&
            (vert.y < rhs.vert.y || (vert.y == rhs.vert.y &&
             vert.z < rhs.vert.z)));
   }

   vec3 vert;
};

// An indexed face
struct Face {
   Face() : v1(-1), v2(-1), v3(-1) {}
   Face(int v1, int v2, int v3) : v1(v1), v2(v2), v3(v3) {}

   bool operator== (const Face &rhs) {
      return (v1 = rhs.v1 && v2 == rhs.v2 && v3 == rhs.v3)
          || (v1 = rhs.v2 && v2 == rhs.v3 && v3 == rhs.v1)
          || (v1 = rhs.v3 && v2 == rhs.v1 && v3 == rhs.v2);
   }

   int v1, v2, v3;
};


struct TriangleSet {
   
   TriangleSet() : numVerts(0) {}

   // All the points in the set
   vector<vec3> verts;
   // Look up the index of a vert by value
   map<Vert, int> vertTable;
   int numVerts;
   // Indexed list of all faces
   vector<Face> faces;
   // For each vert, a list of all faces it occupies
   vector<vector<int> > vertFaces; // index by vert to get a vector of face indexes

   vector<int> freeVerts;
   vector<int> freeFaces;
   
   // If the vert exists, return its index, otherwise 
   // add it and return the new index
   int addVert(const vec3 &v) {
      map<Vert, int>::iterator itr = vertTable.find(v);

      if (itr == vertTable.end()) {
         vertTable[v] = numVerts;
         verts.push_back(v);
         vertFaces.push_back(vector<int>());
         return numVerts++;
      }

      return itr->second;
   }

   void addFace(const vec3 &v1, const vec3 &v2, const vec3 &v3) {
      addFace(addVert(v1), addVert(v2), addVert(v3));
   }

   // Add a face to the set, duplicate faces are not checked
   void addFace(int v1, int v2, int v3) {
      Face face(v1, v2, v3);
     
      if (freeFaces.size() == 0)
         faces.push_back(face);
      else {
         faces[freeFaces.back()] = face;
         freeFaces.pop_back();
      }

      vertFaces[face.v1].push_back(faces.size()-1);
      vertFaces[face.v2].push_back(faces.size()-1);
      vertFaces[face.v3].push_back(faces.size()-1);
   }

   vector<vec3> adjacentVerts(const vec3 &from) {
      map<Vert, int>::iterator itr = vertTable.find(from);
      vector<vec3> ret;

      if (itr != vertTable.end()) {
         vector<int> ndx = adjacentVerts(itr->second);
         for (unsigned i = 0; i < ndx.size(); i++)
            ret.push_back(verts[ndx[i]]);
      }

      return ret;
   }
   
   vector<int> adjacentVerts(int from) {
      set<int> ret;

      for (unsigned i = 0; i < vertFaces[from].size(); i++) {
         const Face &face = faces[vertFaces[from][i]];
         if (face.v1 != -1) {
            ret.insert(face.v1);
            ret.insert(face.v2);
            ret.insert(face.v3);
         }
      }

      ret.erase(from);
      return vector<int>(ret.begin(), ret.end());
   }

   vector<int> getCommonFaces(int v1, int v2) {
      vector<int> commonFaces;

      for (unsigned i = 0; i < vertFaces[v1].size(); i++)
         if (find(vertFaces[v2].begin(), vertFaces[v2].end(), vertFaces[v1][i]) != vertFaces[v2].end())
            commonFaces.push_back(vertFaces[v1][i]);

      return commonFaces;
   }

   void collapseEdge(int v1, int v2, vec3 newPos) {
      // Find all faces that contain both points
      vector<int> commonFaces = getCommonFaces(v1, v2);
      /*for (unsigned i = 0; i < vertFaces[v1].size(); i++)
         if (find(vertFaces[v2].begin(), vertFaces[v2].end(), vertFaces[v1][i]) != vertFaces[v2].end())
            commonFaces.push_back(vertFaces[v1][i]);*/

      // Disable those faces and remove them from each vert's face list
      for (unsigned i = 0; i < commonFaces.size(); i++) {
         faces[commonFaces[i]].v1 = -1;
         freeFaces.push_back(commonFaces[i]);
         vectorRemove(commonFaces[i], vertFaces[v1]);
         vectorRemove(commonFaces[i], vertFaces[v2]);
      }

      // Set all of the faces that use v2 to use v1 instead
      for (unsigned i = 0; i < vertFaces[v2].size(); i++) {
         Face &face = faces[vertFaces[v2][i]];
         if (face.v1 == v2)
            face.v1 = v1;
         else if (face.v2 == v2)
            face.v2 = v1;
         else if (face.v3 == v2)
            face.v3 = v1;
         else
            printf("Could not find vertex in associated face on edge collapse\n");
      }

      // Insert all of v2's faces into v1's face list and delete v2's face list.
      vertFaces[v1].insert(vertFaces[v1].end(), vertFaces[v2].begin(), vertFaces[v2].end());
      vertFaces[v2].clear();

      // Update v1's position
      vertTable.erase(verts[v1]);
      verts[v1] = newPos;
      vertTable[newPos] = v1;
   }

   vec3 faceCentroid(int f) {
      return (verts[faces[f].v1] + verts[faces[f].v2] + verts[faces[f].v3]) / 3.0;
   }

   vec3 calcEdgePoint(int v1, int v2) {
      vector<int> commonFaces = getCommonFaces(v1, v2);
      vec3 ret(0, 0, 0);
      for (unsigned i = 0; i < commonFaces.size(); i++)
         ret = ret + faceCentroid(commonFaces[i]);
      return (verts[v1] + verts[v2] + ret) / (2.0f + commonFaces.size());
   }

   vec3 calcOrigPoint(int v1) {
      vector<int> adj = adjacentVerts(v1);

      vec3 eavg(0, 0, 0);
      for (unsigned i = 0; i < adj.size(); i++) {
         eavg = eavg + lin(verts[v1], verts[adj[i]], 0.5);
      }
      eavg = eavg/(float)adj.size();

      vec3 favg(0, 0, 0);
      for (unsigned i = 0; i < vertFaces[v1].size(); i++) {
         favg = favg + faceCentroid(vertFaces[v1][i]);
      }
      favg = favg/(float)vertFaces[v1].size();

      return (favg + eavg * 2.0f + verts[v1] * (adj.size() - 3.0f)) / (float)adj.size();
   }

   const static int ccorig = 0;

   TriangleSet CCSubDiv() {
      TriangleSet ret;

      for (unsigned i = 0; i < faces.size(); i++) {
         vec3 F = faceCentroid(i);
         vec3 e1 = calcEdgePoint(faces[i].v1, faces[i].v2);
         vec3 e2 = calcEdgePoint(faces[i].v2, faces[i].v3);
         vec3 e3 = calcEdgePoint(faces[i].v3, faces[i].v1);
         vec3 V1 = calcOrigPoint(faces[i].v1);
         vec3 V2 = calcOrigPoint(faces[i].v2);
         vec3 V3 = calcOrigPoint(faces[i].v3);
   
         if (ccorig) {         
            ret.addFace(V1, e1, F);
            ret.addFace(e1, V2, F);
            ret.addFace(V2, e2, F);
            ret.addFace(e2, V3, F);
            ret.addFace(V3, e3, F);
            ret.addFace(e3, V1, F);
         } else {  
            ret.addFace(V1, e1, e3);
            ret.addFace(V2, e2, e1);
            ret.addFace(V3, e3, e2);
            ret.addFace(e1, e2, e3);
         }
         
      }

      return ret;
   }

   // Returns a new triangle set with no invalid verts or faces
   TriangleSet cleanUp() const {
      TriangleSet ts;
      
      for (unsigned i = 0; i < faces.size(); i++) {
         const Face &f = faces[i];
         if (f.v1 != -1)
            ts.addFace(verts[f.v1], verts[f.v2], verts[f.v3]);
      }

      return ts;
   }
};

inline TriangleSet icosahedron()
{
   TriangleSet set;

   const float ratio = (1.0 + sqrt(5.0)) / 2.0;
   const float radius = 1.0 / sqrt(ratio * sqrt(5.0));

   vec3 p11 = vec3(0,  1,  ratio).normalize();
   vec3 p12 = vec3(0,  1, -ratio).normalize();
   vec3 p13 = vec3(0, -1,  ratio).normalize();
   vec3 p14 = vec3(0, -1, -ratio).normalize();

   vec3 p21 = vec3( 1,  ratio, 0).normalize();
   vec3 p22 = vec3( 1, -ratio, 0).normalize();
   vec3 p23 = vec3(-1,  ratio, 0).normalize();
   vec3 p24 = vec3(-1, -ratio, 0).normalize();

   vec3 p31 = vec3( ratio, 0,  1).normalize();
   vec3 p32 = vec3(-ratio, 0,  1).normalize();
   vec3 p33 = vec3( ratio, 0, -1).normalize();
   vec3 p34 = vec3(-ratio, 0, -1).normalize();
   
   set.addFace(p11, p13, p31);
   set.addFace(p13, p11, p32);
   set.addFace(p14, p12, p33);
   set.addFace(p12, p14, p34);
 
   set.addFace(p21, p23, p11);
   set.addFace(p23, p21, p12);
   set.addFace(p24, p22, p13);
   set.addFace(p22, p24, p14);
 
   set.addFace(p31, p33, p21);
   set.addFace(p33, p31, p22);
   set.addFace(p34, p32, p23);
   set.addFace(p32, p34, p24);
 
   set.addFace(p11, p31, p21);
   set.addFace(p11, p23, p32);

   set.addFace(p12, p21, p33);
   set.addFace(p12, p34, p23);

   set.addFace(p13, p22, p31);
   set.addFace(p13, p32, p24);

   set.addFace(p14, p33, p22);
   set.addFace(p14, p24, p34);

   return set;
}

#endif
