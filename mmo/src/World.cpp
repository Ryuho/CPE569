#include "World.h"
#include "Characters.h"
#include "Texture.h"
#include "packet.h"
#include "Constants.h"
#include "GLUtil.h"
#include "Sprite.h"

using namespace constants;

struct WorldData {
   WorldData() {};
   int width, height;
   int ticks;
   float dt;
   vec2 playerMoveDir;
   int arrowTick, specialTick;

   ObjectHolder objs;
   Player player, shadow;
   Texture ground;
   
   sock::Connection conn;

   void init(const char *host, int port);
   void update();
   void draw();
   void processPacket(pack::Packet p);
};

// This pointer points to the current 
WorldData* clientState;
int wHeight;
int wWidth;
GLfloat boarderWidth = .05;

// Interface stubs
void World::init(const char *host, int port) {
   data.reset(new WorldData);
   data->init(host,port);
}
void World::update(int ticks, float dt) { 
   data->ticks = ticks;
   data->dt = dt;
   data->update();
}
void World::draw() { data->draw(); }

void WorldData::init(const char *host, int port)
{
   clientState = this;

   arrowTick = 0;
   specialTick = 0;

   sock::setupSockets();

   // Maybe ask for a hostname here, along with other info (name, class etc.)
   conn = sock::Connection(host, port);

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
	
	wHeight = c.worldHeight;
	wWidth = c.worldWidth;
   if (!conn.select(1000)) {
      printf("Timed out waiting for initalize packet\n");
      exit(-1);
   }
   
   p = pack::readPacket(conn);
   if (p.type != pack::initialize) {
      printf("Expecting initalize, got %d\n", p.type);
      exit(-1);
   }
   
   pack::Initialize i(p);
   if (i.id != c.id || i.type != ObjectType::Player) {
      printf("Bad init packet\n");
      exit(-1);
   }
	
   player = Player(i.id, i.pos, i.dir, playerMaxHp);
   shadow = player;

   printf("Connected to server successfully\nYour id is %d\n", player.id);
}

void World::graphicsInit(int width, int height)
{
   initCharacterResources();

   data->width = width;
   data->height = height;

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
      playerMoveDir.normalize();
      player.move(player.pos + playerMoveDir * dt * playerSpeed, playerMoveDir, true);
		if (player.pos.x > worldWidth)
		{
			player.pos.x = worldWidth;
		}
		else if (player.pos.x < -worldWidth)
		{
			player.pos.x = -worldWidth;
		}
		if (player.pos.y > worldHeight)
		{
			player.pos.y = worldHeight;
		}
		else if (player.pos.y < -worldHeight)
		{
			player.pos.y = -worldHeight;
		}
   } else
      player.stop();

   pack::Position(player.pos, player.dir, player.moving, player.id).makePacket().sendTo(conn);

   objs.updateAll();
}

void WorldData::processPacket(pack::Packet p)
{
	using namespace pack;
   
   if (p.type == position) {
      Position pos(p);
      if(pos.id == player.id) {
         shadow.move(pos.pos, pos.dir, pos.moving != 0);
      } else if(objs.checkObject(pos.id, ObjectType::Player)) {
         Player *p = objs.getPlayer(pos.id);
         if(p)
            p->move(pos.pos, pos.dir, pos.moving != 0);
         else
            printf("Accessing unknown Player %d\n", pos.id);
      } else if (objs.checkObject(pos.id, ObjectType::NPC)) {
         NPC *npc = objs.getNPC(pos.id);
         if(npc) {
            if(!npc->moving && pos.moving) {
               npc->anim->animStart = getTicks();
            }
            npc->pos = pos.pos;
            npc->dir = pos.dir;
            npc->moving = pos.moving != 0;
         }
         else
            printf("Accessing unknown NPC %d\n", pos.id);
      } else if(objs.checkObject(pos.id, ObjectType::Item)) {
         Item *item = objs.getItem(pos.id);
         if(item)
            item->pos = pos.pos;
         else
            printf("Accessing unknown Item %d\n", pos.id);
      }
      else
         printf("client %d: unable to process Pos packet id=%d\n", player.id, pos.id);
   }
   else if (p.type == initialize) {
      Initialize i(p);
      if (i.type == ObjectType::Player && i.id != player.id) {
         objs.addPlayer(Player(i.id, i.pos, i.dir, i.hp));
         
         printf("Added player pos: %.1f %.1f\n", objs.getPlayer(i.id)->pos.x, objs.getPlayer(i.id)->pos.y);
      }
      else if (i.type == ObjectType::Missile) {
         objs.addMissile(Missile(i.id, i.subType, i.pos, i.dir));
      } 
      else if (i.type == ObjectType::NPC) {
         objs.addNPC(NPC(i.id, i.subType, i.pos, i.dir, false));
         printf("Added NPC %d \n", i.id);
      }
      else if (i.type == ObjectType::Item) {
         objs.addItem(Item(i.id, i.subType, i.pos));
         printf("Added Item %d \n", i.id);
      }
   } 
   else if (p.type == signal) {
      Signal sig(p);
      if (sig.sig == Signal::remove) {
         if(sig.val == this->player.id)
            printf("\n\n!!! Disconnected from server !!!\n\n");
         else {
            objs.removeObject(sig.val);
            printf("Object %d disconnected\n", sig.val);
         }
      } else if (sig.sig == Signal::changeRupee) {
         player.rupees = sig.val;
         printf("rupees = %d\n", player.rupees);
      } else if (sig.sig == Signal::changeExp) {
         player.exp = sig.val;
         printf("exp = %d\n", player.exp);
      } else
         printf("Unknown signal (%d %d)\n", sig.sig, sig.val);
   } 
   else if (p.type == arrow) {
	   Arrow ar(p);
		objs.addMissile(Missile(ar.id, MissileType::Arrow, ar.orig, ar.direction));
	}
   else if (p.type == healthChange) {
      HealthChange hc(p);
      if (hc.id == player.id) {
         player.hp = hc.hp;
         shadow.hp = hc.hp;
      } else {
         objs.getPlayer(hc.id)->hp = hc.hp;
      }
   }
   else
      printf("Unknown packet type=%d size=%d\n", p.type, p.data.size());
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
	
	glColor3ub(0, 0, 0);
	//glLineWidth (boarderWidth);
	glBegin(GL_LINES);
	glVertex2f(wWidth, wHeight);
	glVertex2f(-wWidth, wHeight);
	glVertex2f(wWidth, wHeight);
	glVertex2f(wWidth, -wHeight);
	glVertex2f(wWidth, -wHeight);
	glVertex2f(-wWidth, -wHeight);
	glVertex2f(-wWidth, -wHeight);
	glVertex2f(-wWidth, wHeight);
	glEnd();
   /*glColor4ub(255,255,255,128);
   shadow.draw();
   glColor4ub(255,255,255,255);*/
	glColor3ub(255, 255, 255);

   objs.drawAll();
   
   player.draw();
}

void World::move(mat::vec2 dir)
{
   data->playerMoveDir = dir;
}

void World::shootArrow(mat::vec2 dir)
{
	//pack::Arrow ar(data->player.pos, dir - vec2(data->width/2,data->height/2), clientState->player.id);
	pack::Arrow ar(dir - vec2(data->width/2,data->height/2), clientState->player.id);	
	ar.makePacket().sendTo(clientState->conn);
	//printf("arrow packet sent!\n");      
	/**if (data->ticks - data->arrowTick > arrowCooldown) {
      Missile m(game::getTicks()); // using get ticks here is a dumb hack, use newId() on the server
      m.init(data->player.pos, dir - vec2(data->width/2,data->height/2), Missile::Arrow);
      data->objs.addMissile(m);
      data->arrowTick = data->ticks;
   }**/
}

void World::doSpecial()
{
	pack::Signal sig(pack::Signal::special, clientState->player.id);
   sig.makePacket().sendTo(clientState->conn);	
	printf("special packet sent!\n");   
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

void World::rightClick(vec2 mousePos)
{
   vec2 clickPos = clientState->player.pos + mousePos 
      - vec2(data->width/2,data->height/2);
   pack::Click(clickPos, clientState->player.id).makePacket().sendTo(data->conn);
   printf("Clicked <%5.1f %5.1f>\n", mousePos.x, mousePos.y);
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
   pack::Signal(pack::Signal::hurtme).makePacket().sendTo(data->conn);
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
