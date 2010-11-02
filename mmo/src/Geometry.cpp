#include "Geometry.h"
#include <stdio.h>
using namespace geom;


void Circle::init(vec2 pos, float radius) {
   this->radius = radius;
   this->pos = pos;
}

bool Circle::collision(Geometry *other) {
   return other->collision(this);
}

bool Circle::collision(Circle *other) {
   return mat::dist(pos, other->pos) < other->radius + radius;
}

bool Circle::collision(Square *other) {
   printf("unimplemented\n");
   return true; //TODO
}

bool Circle::collision(Plane *other) {
   printf("unimplemented\n");
   return true; //TODO
}

void Square::init(vec2 bottomLeft, vec2 size) {
   this->bottomLeft = bottomLeft;
   this->size = size;
}

bool Square::collision(Geometry *other) {
   return other->collision(this);
}

bool Square::collision(Circle *other) {
   printf("unimplemented\n");
   return true; //TODO
}

bool Square::collision(Square *other) {
   printf("unimplemented\n");
   return true; //TODO
}

bool Square::collision(Plane *other) {
   printf("unimplemented\n");
   return true; //TODO
}

void Plane::init(vec2 pos, float slope) {
   this->pos = pos;
   this->slope = slope;
}

bool Plane::collision(Geometry *other) {
   return other->collision(this);
}

bool Plane::collision(Circle *other) {
   printf("unimplemented\n");
   return true; //TODO
}

bool Plane::collision(Square *other) {
   printf("unimplemented\n");
   return true; //TODO
}

bool Plane::collision(Plane *other) {
   printf("unimplemented\n");
   return true; //TODO
}
