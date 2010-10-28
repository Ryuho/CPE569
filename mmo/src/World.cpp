#include "World.h"
#include "Characters.h"
#include "Texture.h"
#include "packet.h"
#include "Constants.h"
#include "GameUtil.h"
#include "GLUtil.h"

using namespace constants;

struct WorldData {
   WorldData() : player(0), shadow(0) {}
   int width, height;
   int ticks;
   float dt;
   vec2 playerMoveDir;
   int arrowTick, specialTick;

   ObjectHolder objs;
   //map<int, int> idToIndex;
   //vector<Player> players;
   //vector<Missile> missiles;
   //vector<Item> items;
   //vector<NPC> npcs;
   Player player, shadow;
   Texture ground;
   
   sock::Connection conn;

   void init();
   void update();
   void draw();
   void processPacket(pack::Packet p);
};

// This pointer points to the current 
WorldData* clientState;

// Interface stubs
void World::init() {
   data.reset(new WorldData);
   data->init();
}
void World::update(int ticks, float dt) { 
   data->ticks = ticks;
   data->dt = dt;
   data->update();
}
void World::draw() { data->draw(); }

void WorldData::init()
{
   clientState = this;

   arrowTick = 0;
   specialTick = 0;

   sock::setupSockets();

   // Maybe ask for a hostname here, along with other info (name, class etc.)
   conn = sock::Connection("localhost", 27027);

   if (!conn) {
      printf("Could not connect!\n");
      exit(-1);
   }

   if (!conn.select(1000)) {
      printf("Timed out waiting for server\n");
      exit(-1);
   }

   pack::Packet p = pack::readPacket(conn);
   if (p.type != pack::connect) {
      printf("Expected connect packet for handshake, got: %d\n", p.type);
      exit(-1);
   }
   
   pack::Connect c(p);
   player.id = c.id;
   shadow.id = c.id;

   printf("Connected to server successfully\n");
}

void World::graphicsInit(int width, int height)
{
   initCharacterResources();

   data->width = width;
   data->height = height;
   data->player.setPos(vec2(width/2, height/2));
   data->shadow.setPos(vec2(width/2, height/2));

   data->ground = fromTGA("grass.tga");
}

void WorldData::update()
{
   while (conn.select()) {
      if (conn) {
         processPacket(pack::readPacket(conn));
      } else {
         printf("server disconnected!\n");
         exit(-1);
      }
   }

   if (playerMoveDir.length() > 0.0) {
      player.moveTo(player.pos + playerMoveDir * dt * playerSpeed);
      pack::Pos pos(player.pos + playerMoveDir * dt * playerSpeed);
      pos.makePacket().sendTo(conn);
   } else if (player.moving) {
      // todo: Send a stopping signal to server
      player.moving = false;
   }

   objs.updateAll();
}

void WorldData::processPacket(pack::Packet p)
{
   using namespace pack;
   if (p.type == pos) {
      Pos pos(p);
      if (pos.id == player.id) {
         shadow.setPos(pos.v);
      } else {
         objs.getPlayer(pos.id).moveTo(pos.v);
      }
   } else if (p.type == signal) {
      Signal sig(p);
      if (sig.sig == Signal::disconnect) {
         objs.removeObject(sig.val);
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

   glColor4ub(255,255,255,128);
   shadow.draw();
   glColor4ub(255,255,255,255);

   objs.drawAll();
}

void World::move(mat::vec2 dir)
{
   data->playerMoveDir = dir;
}

void World::shootArrow(mat::vec2 dir)
{
   if (data->ticks - data->arrowTick > arrowCooldown) {
      Missile m(game::getTicks()); // using get ticks here is a dumb hack, use newId() on the server
      m.init(data->player.pos, dir - vec2(data->width/2,data->height/2), Missile::Arrow);
      data->objs.addMissile(m);
      data->arrowTick = data->ticks;
   }
}

void World::doSpecial()
{
   /*if (data->ticks - data->specialTick > specialCooldown) {
      for (int i = 0; i < numArrows; i++) {
         float t = i/(float)numArrows;
         Missile arrow;
         arrow.init(data->player.pos, vec2(cos(t*2*PI), sin(t*2*PI)), Missile::Arrow);
         data->missiles.push_back(arrow);
         data->specialTick = data->ticks;
      }
   }*/
}

void World::spawnItem()
{
/*
   Item i;
   i.init(data->player.pos + vec2(100, 100), (Item::Type) (rand() % ((int)Item::MaxItem)));
   data->items.push_back(i);
   */
}

void World::spawnNPC()
{
/*
   NPC n;
   //float t = rand()/(float)RAND_MAX;
   //vec2 _pos = vec2(cos(t*2*PI), sin(t*2*PI)) * 1600;

   int minv, maxv, midv;
   minv = 700;
   maxv = 1200;
   midv = 400;

   vec2 _pos = vec2(rand()%maxv - minv, rand()%maxv - minv);
   if(_pos.x >= 0.0 && _pos.x < midv)
      _pos.x = midv;
   else if(_pos.x < 0.0 && _pos.x > -midv)
      _pos.x = -midv;
   if(_pos.y >= 0.0 && _pos.y < midv)
      _pos.y = midv;
   else if(_pos.y < 0.0 && _pos.y > -midv)
      _pos.y = -midv;
   //n.init(data->player.pos + _pos, NPC::Ganon);
   n.init(data->player.pos + _pos, (NPC::Type) (rand() % ((int)NPC::MaxNPC)));
   data->npcs.push_back(n);
   */
}

// Global accessor functions

namespace game {

int getTicks()
{
   return clientState->ticks;
}

float getDt()
{
   return clientState->dt;
}

Player &getPlayer()
{
   return clientState->player;
}

} // end client namespace
