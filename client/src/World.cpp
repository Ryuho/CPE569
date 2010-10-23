#include "World.h"
#include "Characters.h"
#include "Texture.h"

struct WorldData {
   int width, height;
   int ticks;
   float dt;
   vec2 playerMoveDir;
   int arrowTick, specialTick;

   vector<Missile> missiles;
   vector<Item> items;
   Player player;
   Texture ground;

   void update(int ticks, float dt);
   void draw();
};

// Interface stubs
void World::update(int ticks, float dt) { data->update(ticks, dt); }
void World::draw() { data->draw(); }

// Constants
namespace {
   float playerSpeed = 400; // Number of pixels per second the player can move
   int numArrows = 30; // Number of arrows that are fired in the special attack
   int arrowCooldown = 150; // Number of ms between arrow shots
   int specialCooldown = 2000; // ms between special attacks
}

void World::init(int width, int height)
{
   initCharacterResources();
   data.reset(new WorldData());

   data->arrowTick = 0;
   data->specialTick = 0;

   data->width = width;
   data->height = height;
   data->player.update(vec2(width/2, height/2), vec2(0.0, -1.0), false);

   data->ground = fromTGA("grass.tga");
}

void WorldData::update(int ticks, float dt)
{
   this->ticks = ticks;
   this->dt = dt;

   if (playerMoveDir.length() > 0.0)
      player.update(player.pos + player.dir * dt * playerSpeed, playerMoveDir, true);
   else
      player.moving = false;

   for (unsigned i = 0; i < missiles.size(); i++) {
      missiles[i].update(dt);
      if (!missiles[i].alive) {
         missiles[i] = missiles.back();
         missiles.pop_back();
         i--;
      }
   }
   
   for (unsigned i = 0; i < items.size(); i++) {
      items[i].update(dt, player);
      if (!items[i].alive) {
         items[i] = items.back();
         items.pop_back();
         i--;
      }
   }
}

void WorldData::draw()
{
   glColor3ub(255, 255, 255);
   glDisable(GL_DEPTH_TEST);
   glEnable(GL_TEXTURE_2D);

   ground.bind();

   glBegin(GL_QUADS);
   
   float xoffset = player.pos.x / (float) width;
   float yoffset = player.pos.y / (float) height;
   
   glTexCoord2f(xoffset,yoffset);
   glVertex2f(0,0);
   glTexCoord2f(1+xoffset,yoffset);
   glVertex2f(width, 0);
   glTexCoord2f(1+xoffset,1+yoffset);
   glVertex2f(width, height);
   glTexCoord2f(xoffset,1+yoffset);
   glVertex2f(0, height);
   glEnd();
   
   glTranslatef(-player.pos.x + width/2, -player.pos.y + height/2, 0.0);

   player.draw();

   for (unsigned i = 0; i < missiles.size(); i++)
      missiles[i].draw();
      
   for (unsigned i = 0; i < items.size(); i++)
      items[i].draw();
}

void World::move(mat::vec2 dir)
{
   data->playerMoveDir = dir;
}

void World::shootArrow(mat::vec2 dir)
{
   if (data->ticks - data->arrowTick > arrowCooldown) {
      Missile m;
      m.init(data->player.pos, dir - vec2(data->width/2,data->height/2), Missile::Arrow);
      data->missiles.push_back(m);
      data->arrowTick = data->ticks;
   }
}

void World::doSpecial()
{
   if (data->ticks - data->specialTick > specialCooldown) {
      for (int i = 0; i < numArrows; i++) {
         float t = i/(float)numArrows;
         Missile arrow;
         arrow.init(data->player.pos, vec2(cos(t*2*PI), sin(t*2*PI)), Missile::Arrow);
         data->missiles.push_back(arrow);
         data->specialTick = data->ticks;
      }
   }
}

void World::spawnItem()
{
   Item i;
   i.init(data->player.pos + vec2(100, 100), Item::Rupee);
   data->items.push_back(i);
}
