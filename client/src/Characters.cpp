#include "Characters.h"
#include "Sprite.h"

namespace {
   Texture spriteTex;
   Sprite sprites32;
   Sprite sprites64;
   Sprite sprites16x32;
   Sprite sprites16;
   Sprite sprites8;

   int spriteZoom = 4;

   bool initalized = false;
};


void initCharacterResources()
{
   if (!initalized) {
      TexOptions opt;
      opt.min_filter = GL_NEAREST;
      opt.mag_filter = GL_NEAREST;
      spriteTex = fromTGA("character.tga", opt);
      sprites32 = Sprite(spriteTex, 32, 32);
      sprites64 = Sprite(spriteTex, 64, 64);
      sprites16x32 = Sprite(spriteTex, 16, 32);
      sprites16 = Sprite(spriteTex, 16, 16);
      sprites8 = Sprite(spriteTex, 8, 8);
      initalized = true;
   }
}

void Player::update(mat::vec2 pos, mat::vec2 dir, bool moving)
{
   if (!this->moving && moving) {
      animStart = SDL_GetTicks();
   }

   this->pos = pos;
   if (dir.length() > 0.0)
      this->dir = normalize(dir);
   else
      this->dir = vec2(0.0, 1.0);
   this->moving = moving;
}

void Player::draw()
{
   int frame, adir;

   if (moving)
      frame = ((SDL_GetTicks() - animStart) / 100) % 8;
   else
      frame = 0;

   if (dir.y > 0.8)
      adir = 2;
   else if (dir.y < -0.8)
      adir = 4;
   else if (dir.x > 0)
      adir = 3;
   else
      adir = 1;

   glPushMatrix();
   glTranslatef(pos.x, pos.y, 0.0);
   glScalef(spriteZoom, spriteZoom, 1.0);
   if (moving)
      sprites32.draw(frame, adir);
   else
      sprites32.draw(adir-1, 0);
   glPopMatrix();
}

void Missile::init(mat::vec2 pos, mat::vec2 dir, Missile::Type type)
{
   start = pos;
   if (dir.length() > 0.0)
      this->dir = normalize(dir);
   else
      this->dir = vec2(0.0, 1.0);
   this->type = type;

   this->pos = start + this->dir * 75.0;

   alive = true;
}

void Missile::update(float fdt)
{
   if (alive) {
      float speed = 800.0;

      pos = pos + dir * speed * fdt;

      if (dist(start, pos) > 500)
         alive = false;
   }
}

void Missile::draw()
{
   if (alive) {
      glPushMatrix();
      glTranslatef(pos.x, pos.y, 0.0);
      glScalef(spriteZoom, spriteZoom, 1.0);
      glRotatef(toDeg(atan2(dir.y, dir.x)), 0, 0, 1);
      
      sprites16.draw(9,0); // firebolt
      //sprites16.draw(8,0); // arrow

      //sprites16x32.draw(18,1); // pillar

      //sprites32.draw(12,2); // mini trident
      //sprites32.draw(11,0); // small fireball
      //sprites32.draw(6,0); // ball?
      //sprites32.draw(5,0); // bomb (dont rotate)
      //sprites32.draw(7,0); // boomerang (dont rotate, animation 7 - 10)
      //sprites32.draw(12,0); // strange 4 orb thing. rotate and animate 12-13
      //sprites64.draw(7,0); // big ass spear
      glPopMatrix();
   }
}

void Item::init(vec2 pos, Type type)
{
    this->pos = pos;
    this->type = type;
    this->alive = true;
}

void Item::update(float fdt, Player &player)
{
  if (alive) {
      //if within 40 pixels make not alive
      if (dist(player.pos, pos) < 40)
         alive = false;
   }
}

void Item::draw()
{
  if (alive) {
      glPushMatrix();
      glTranslatef(pos.x, pos.y, 0.0);
      glScalef(spriteZoom, spriteZoom, 1.0);
      //glRotatef(toDeg(atan2(dir.y, dir.x)), 0, 0, 1);
      
      sprites16.draw(13,0); // green rupee 1

      glPopMatrix();
   }  
}









void NPC::init(vec2 pos, Type type)
{
    this->type = type;
    this->animStart = 0;
    this->pos = pos;
    this->moving = false;
    this->alive = true;
    this->dir = vec2(0, 0);
}

void NPC::update(float fdt, Player &player)
{
   if(alive) {
      float speed = 800;
      vec2 dist = to(pos, player.pos);
      dir = normalize(dist);
      if(dist.length() > 40) {         this->pos = dir*speed*fdt + pos;
      }
   }
}

void NPC::draw()
{
  if (alive) {
      glPushMatrix();
      glTranslatef(pos.x, pos.y, 0.0);
      glScalef(spriteZoom, spriteZoom, 1.0);
      //glRotatef(toDeg(atan2(dir.y, dir.x)), 0, 0, 1);
      
      sprites16x32.draw(6, 9); // green rupee 1

      glPopMatrix();
   }  
}
