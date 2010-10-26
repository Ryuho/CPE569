#include "World.h"
#include "Characters.h"
#include "Texture.h"
#include "packet.h"
#include <map>

struct WorldData {
   int width, height;
   int ticks;
   float dt;
   vec2 playerMoveDir;
   int arrowTick, specialTick;

   int id;

   map<int, int> idToIndex;
   vector<Player> players;
   vector<Missile> missiles;
   vector<Item> items;
   vector<NPC> npcs;
   Player player, shadow;
   Texture ground;
   
   sock::Connection conn;

   void update(int ticks, float dt);
   void draw();
   void processPacket(pack::Packet p);
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

void World::init()
{
   data.reset(new WorldData());
   data->arrowTick = 0;
   data->specialTick = 0;
   
   sock::setupSockets();
   
   data->conn = sock::Connection("localhost", 27027);

   if (!data->conn) {
      printf("Could not connect!\n");
      exit(-1);
   }

   if (!data->conn.select(1000)) {
      printf("Timed out waiting for server\n");
      exit(-1);
   }

   pack::Packet p = pack::readPacket(data->conn);
   if (p.type != pack::connect) {
      printf("Expected connect packet for handshake, got: %d\n", p.type);
      exit(-1);
   }
   
   pack::Connect c(p);
   data->id = c.id;

   printf("Connected to server successfully\n");
}

void World::graphicsInit(int width, int height)
{
   initCharacterResources();

   data->width = width;
   data->height = height;
   data->player.update(vec2(width/2, height/2), vec2(0.0, -1.0), false);
   data->shadow.update(vec2(width/2, height/2), vec2(0.0, -1.0), false);

   data->ground = fromTGA("grass.tga");
}

void WorldData::update(int ticks, float dt)
{
   this->ticks = ticks;
   this->dt = dt;

   while (conn.select()) {
      if (conn) {
         processPacket(pack::readPacket(conn));
      } else {
         printf("server disconnected!\n");
         exit(-1);
      }
   }

   if (playerMoveDir.length() > 0.0) {
      player.update(player.pos + playerMoveDir * dt * playerSpeed, playerMoveDir, true);
      pack::Pos pos(player.pos + playerMoveDir * dt * playerSpeed);
      pos.makePacket().sendTo(conn);
   } else
      player.moving = false;

   /*if (playerMoveDir.length() > 0.0) 
      player.update(player.pos + player.dir * dt * playerSpeed, playerMoveDir, true);
   else
      player.moving = false;*/

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
  
   /*for (unsigned i = 0; i < npcs.size(); i++) {
      npcs[i].update(dt, player);
      if (!npcs[i].alive) {
         npcs[i] = npcs.back();
         npcs.pop_back();
         i--;
      }
   }*/
}

void WorldData::processPacket(pack::Packet p)
{
   using namespace pack;
   if (p.type == pos) {
      Pos pos(p);
      if (pos.id == id) {
         shadow.update(pos.v, ticks);
      } else {
         map<int,int>::iterator itr = idToIndex.find(pos.id);
         if (itr == idToIndex.end()) {
            printf("adding new player %d because of position update\n", pos.id);
            players.push_back(Player());
            players.back().update(pos.v, ticks);
            idToIndex[pos.id] = players.size()-1;
         } else {
            players[idToIndex[pos.id]].update(pos.v, ticks);
         }
      }
   } else if (p.type == signal) {
      Signal sig(p);
      if (sig.sig == Signal::disconnect) {
         map<int,int>::iterator itr = idToIndex.find(sig.val);
         if (itr != idToIndex.end()) {
            players[idToIndex[sig.val]].alive = false;
         }
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

   glColor4ub(255,255,255,50);
   shadow.draw();
   glColor4ub(255,255,255,255);

   for (unsigned i = 0; i < players.size(); i++)
      players[i].draw();

   for (unsigned i = 0; i < missiles.size(); i++)
      missiles[i].draw();
      
   for (unsigned i = 0; i < items.size(); i++)
      items[i].draw();

   for (unsigned i = 0; i < npcs.size(); i++)
      npcs[i].draw();
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
