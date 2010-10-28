#include "GameServer.h"
#include <cstdio>

using namespace pack;

GameServer *serverState;

GameServer::GameServer(ConnectionManager &cm) : cm(cm)
{
   serverState = this;
   ticks = 0;
   dt = 0;
   Player p(5);
   p.update();
   p.draw();
}

void GameServer::newConnection(int id)
{
   printf("New connection: %d\n", id);
   cm.sendPacket(Connect(id), id);
}

void GameServer::disconnect(int id)
{
   printf("Client %d disconnected\n", id);
   cm.broadcast(Signal(Signal::disconnect, id).makePacket());
}

void GameServer::processPacket(pack::Packet p, int id)
{
   if (p.type == pack::pos) {
      Pos pos(p);
      pos.id = id;
      cm.broadcast(pos);
   }
}

void GameServer::update(int ticks)
{

}

namespace game {

Player &getPlayer()
{
   assert(false);
   static Player p(0);
   return p;
}

int getTicks()
{
   return 0;
}

float getDt()
{
   return 0.0;
}

} // end game namespace

// empty stubs of character graphics-related functions

void initCharacterResources() {}
void Player::draw() {}
void Missile::draw() {}
void Item::initGraphics() {}
void Item::draw() {}
void NPC::initGraphics() {}
void NPC::resetAnimation() {}
void NPC::draw() {}
