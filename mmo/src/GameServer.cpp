#include "GameServer.h"

using namespace pack;

GameServer::GameServer(ConnectionManager &cm) : cm(cm)
{

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
