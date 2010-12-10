#include "World.h"
#include "Characters.h"
#include "Texture.h"
#include "Packets.h"
#include "Constants.h"
#include "GLUtil.h"
#include "Sprite.h"
#include "FreeType.h"

using namespace constants;

struct WorldData {
   WorldData() 
      : ticks(0), width(0), height(0), dt(0.0f), arrowTick(0), 
      specialTick(0), drawEverything(true), drawUpdatedOnly(false) {};
   int width, height;
   int ticks;
   float dt;
   vec2 playerMoveDir;
   int arrowTick, specialTick;
   bool drawEverything;
   bool drawUpdatedOnly;

   freetype::font_data mono;

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
WorldData* clientState = 0;
int wHeight = 0;
int wWidth = 0;
const float boarderWidth = 0.05f;

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
void World::draw() { 
   data->draw(); 
}

void setUpBoarder()
{
	for(int i = 0, j = 0; i < wHeight+41; i = i + 41, j++)
	{
		clientState->objs.border[Direction::Right].push_back(Item(0, ItemType::Teleportor, vec2(wWidth, i)));
		clientState->objs.border[Direction::Right].push_back(Item(0, ItemType::Teleportor, vec2(wWidth, -i)));
		clientState->objs.border[Direction::Left].push_back(Item(0, ItemType::Teleportor, vec2(-wWidth, i)));
		clientState->objs.border[Direction::Left].push_back(Item(0, ItemType::Teleportor, vec2(-wWidth, -i)));
	}
	for(int i = 0, j = wWidth / 10 + 1; i < wWidth+41; i = i + 41, j++)
	{
		clientState->objs.border[Direction::Up].push_back(Item(0, ItemType::Teleportor, vec2(i, wHeight)));
		clientState->objs.border[Direction::Up].push_back(Item(0, ItemType::Teleportor, vec2(-i, wHeight)));
		clientState->objs.border[Direction::Down].push_back(Item(0, ItemType::Teleportor, vec2(i, -wHeight)));
		clientState->objs.border[Direction::Down].push_back(Item(0, ItemType::Teleportor, vec2(-i, -wHeight)));
	}
}

void WorldData::init(const char *host, int port)
{
   clientState = this;
   clientState->ticks = getTicks();
   clientState->dt = 0;

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
   if (p.type != PacketType::connect) {
      printf("Expected connect packet for handshake, got: %d\n", p.type);
      exit(-1);
   }
   
   pack::Connect c(p);
	
	wHeight = c.worldHeight / 2;
	wWidth = c.worldWidth / 2;
   if (!conn.select(1000)) {
      printf("Timed out waiting for initalize packet\n");
      exit(-1);
   }
   
   p = pack::readPacket(conn);
   if (p.type != PacketType::initialize) {
      printf("Expecting initalize, got %d\n", p.type);
      exit(-1);
   }
   
   pack::Initialize i(p);
   if (i.id != c.id || i.type != ObjectType::Player) {
      printf("Bad init packet\n");
      exit(-1);
   }
	
   player = Player(i.id, i.pos, i.dir, playerMaxHp);
   player.exp = 0;
   player.rupees = 0;
   setUpBoarder();
   shadow = player;
   drawEverything = false;
   drawUpdatedOnly = true;

   printf("Connected to server successfully\nYour id is %d\n", player.getId());
}

void World::graphicsInit(int width, int height)
{
   initCharacterResources();

   data->width = width;
   data->height = height;

   data->ground = fromTGA("grass.tga");
   
   data->mono.init("DejaVuSansMono.ttf", 12);
}

void WorldData::update()
{
   bool wasMoving = player.moving;
   while (conn.select()) {
      if (conn) {
         processPacket(pack::readPacket(conn));
      } else {
         printf("server disconnected!\n");
         exit(-1);
      }
   }

   if (playerMoveDir.length() > 0.0f) {
      playerMoveDir.normalize();
      player.move(player.pos + playerMoveDir * dt * playerSpeed, 
         playerMoveDir, true);
		if (player.pos.x > wWidth - player.getRadius()) {
         player.pos.x = wWidth - player.getRadius();
		}
		else if (player.pos.x < -wWidth + player.getRadius()) {
			player.pos.x = -wWidth + player.getRadius();
		}

		if (player.pos.y > wHeight - player.getRadius()) {
			player.pos.y = wHeight - player.getRadius();
		}
		else if (player.pos.y < -wHeight + player.getRadius()) {
			player.pos.y = -wHeight + player.getRadius();
		}
   } 
   else
      player.moving = false;


   if(player.moving || wasMoving)
      pack::Position(player.pos, player.dir, player.moving, 
         player.getId()).makePacket().sendTo(conn);

   objs.updateAll();
   //if(pushed)
   //   printf("5 %0.1f %0.1f\n\n", player.pos.x, player.pos.y);
}

void WorldData::processPacket(pack::Packet p)
{
	using namespace pack;
   
	if (p.type == PacketType::position) {
      Position pos(p);
      if(pos.id == player.getId()) { //ignore
      } else if(objs.contains(pos.id, ObjectType::Player)) {
         Player *obj = objs.getPlayer(pos.id);
         objs.move(obj, pos.pos);
         obj->move(pos.pos, pos.dir, pos.moving != 0);
      } else if (objs.contains(pos.id, ObjectType::NPC)) {
         NPC *obj = objs.getNPC(pos.id);
         objs.move(obj, pos.pos);
         obj->move(pos.pos, pos.dir, pos.moving != 0);
      } else if(objs.contains(pos.id, ObjectType::Item)) {
         Item *obj = objs.getItem(pos.id);
         objs.move(obj, pos.pos);
         obj->move(pos.pos);
      } else if(objs.contains(pos.id, ObjectType::Missile)) {
         Missile *obj = objs.getMissile(pos.id);
         objs.move(obj, pos.pos);
         obj->move(pos.pos, pos.dir);
      }
      //else
      //   printf("client %d: unable to process Pos packet id=%d\n", 
      //      player.getId(), pos.id);
   }
   else if (p.type == PacketType::teleport) {
      Teleport tele(p);
      printf("Teleported\n");
      playerMoveDir = vec2();
      shadow.move(tele.pos, player.dir, false);
      player.move(tele.pos, player.dir, false);
   }
   else if (p.type == PacketType::initialize) {
      Initialize i(p);
      if (i.type == ObjectType::Player) {
         if(i.id == player.getId()) {
            player.pos = i.pos;
            player.dir = i.dir;
            player.hp = i.hp;
            printf("*******Initialized Self %d <%0.1f, %0.1f>\n", 
               i.id, i.pos.x, i.pos.y);
         } 
         else {
            objs.add(new Player(i.id, i.pos, i.dir, i.hp));
            printf("Added Player %d <%0.1f, %0.1f>\n", i.id, i.pos.x, i.pos.y);
         }
      }
      else if (i.type == ObjectType::Missile) {
         objs.add(new Missile(i.id, i.subType, i.pos, i.dir));
      }
      else if (i.type == ObjectType::NPC) {
		  if(!objs.contains(i.id, i.type))
		  {
			objs.add(new NPC(i.id, i.subType, i.hp, i.pos, i.dir, false));
			printf("Added NPC %d hp=%d <%0.1f, %0.1f>\n", i.id, i.hp, i.pos.x, i.pos.y);
		  }
		 else
		 {
			 NPC *obj = objs.getNPC(i.id);
          obj->deserialize(i);
			 objs.move(obj, i.pos);
          obj->move(i.pos, i.dir, obj->moving);
		 }
      }
      else if (i.type == ObjectType::Item) {
         objs.add(new Item(i.id, i.subType, i.pos));
         printf("Added Item %d <%0.1f, %0.1f>\n", i.id, i.pos.x, i.pos.y);
      }
      else
         printf("Error: Unknown initialize type %d\n", i.type);
   } 
   else if (p.type == PacketType::signal) {
      Signal sig(p);
      if (sig.sig == Signal::remove) {
         if(sig.val == this->player.getId())
            printf("\n\n!!! Disconnected from server !!!\n\n");
         else {
            if(objs.contains(sig.val)) {
               int type = objs.get(sig.val)->getType();
               if(type != ObjectType::Missile)
                  printf("Removed %s %d %d\n", 
                     type == ObjectType::Item ? "Item" :
                     type == ObjectType::NPC ? "NPC" :
                     type == ObjectType::Player ? "Player" :
                     "Unknown",
                     sig.val, type);
            }
            objs.remove(sig.val);
            //printf("Object %d disconnected\n", sig.val);
         }
      } else if (sig.sig == Signal::changeRupee) {
         player.rupees = sig.val;
         printf("rupees = %d\n", player.rupees);
      } else if (sig.sig == Signal::changeExp) {
         player.exp = sig.val;
         printf("exp = %d\n", player.exp);
      } else if(sig.sig == Signal::setPvp) {
         printf("Error: Player recieved setPvp signal\n");
      } else
         printf("Unknown signal (%d %d)\n", sig.sig, sig.val);
   } 
   else if (p.type == PacketType::healthChange) {
      HealthChange hc(p);
      if (hc.id == player.getId()) {
         player.hp = hc.hp;
         shadow.hp = hc.hp;
      } 
      else if(objs.contains(hc.id, ObjectType::Player)) {
         objs.getPlayer(hc.id)->hp = hc.hp;
      } 
      else if(objs.contains(hc.id, ObjectType::NPC)) {
         objs.getNPC(hc.id)->hp = hc.hp;
      }
      else{
         printf("Error: Health change on id %d\n", hc.id);
      }
        
   }
   else if(p.type == PacketType::changePvp) {
      Pvp pvpPacket(p);
      if(pvpPacket.id == this->player.getId()) {
         getPlayer().pvp = pvpPacket.isPvpMode != 0;
         if(getPlayer().pvp)
            printf("You are in Pvp Mode\n");
         else
            printf("You are NOT in Pvp Mode\n");
      }
      else if(objs.contains(pvpPacket.id, ObjectType::Player)) {
         Player &play = *objs.getPlayer(pvpPacket.id);
         play.pvp = pvpPacket.isPvpMode != 0;
         if(play.pvp)
            printf("Player %d is in Pvp Mode\n", play.getId());
         else
            printf("Player %d is NOT in Pvp Mode\n", play.getId());
      }
      else
         printf("Error: Unknown player %d for pvp packet\n", pvpPacket.id);
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

   if(drawEverything)
      objs.drawAll(drawUpdatedOnly);
   else
      objs.drawAll(player.pos, drawUpdatedOnly);
   
   player.draw();
   
   freetype::print(mono, 25, 65, "Experience: %d        Rupees: %d\n\nPos: %5.0f %5.0f", 
      player.exp, player.rupees, player.pos.x, player.pos.y);
}

void World::move(mat::vec2 dir)
{
   data->playerMoveDir = dir;
}

void World::shootArrow(mat::vec2 dir)
{
	//pack::Arrow ar(data->player.pos, dir - vec2(data->width/2,data->height/2), clientState->player.id);
	pack::Arrow ar(dir - vec2(data->width/2,data->height/2));	
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
	pack::Signal sig(pack::Signal::special, clientState->player.getId());
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
   pack::Click(clickPos).makePacket().sendTo(data->conn);
   printf("Clicked <%5.1f %5.1f>\n", clickPos.x, clickPos.y);
}

void World::hurtMe()
{
   pack::Signal(pack::Signal::hurtme).makePacket().sendTo(data->conn);
}

void World::togglePvp()
{
   getPlayer().pvp = !getPlayer().pvp;
   pack::Signal(pack::Signal::setPvp, getPlayer().pvp).makePacket()
      .sendTo(data->conn);
}

void World::toggleDrawAll()
{
   data->drawEverything = !data->drawEverything;
   printf("Draw Everything: %s\n", data->drawEverything ? "on" : "off");
}

void World::toggleDrawUpdated()
{
   data->drawUpdatedOnly = !data->drawUpdatedOnly;
   printf("Draw Only Updated: %s\n", data->drawUpdatedOnly ? "on" : "off");
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
