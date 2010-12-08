#include "GameServer.h"
#include "Packets.h"
#include "ServerUtil.h"
#include "Constants.h"
#include <cstdio>

void GameServer::newServerConnection(int id) 
{ 
   printf("Single Game Server Implementation!!!\n");
   printf("Forcing disconnect to remote server.\n");
   std::map<int, int>::iterator iter = cm.idToServerIndex.find(id);
   if(iter != cm.idToServerIndex.end())
      cm.removeServerAt(iter->second);
   else
      exit(EXIT_FAILURE); 
}

void GameServer::serverDisconnect(int id) 
{
   printf("Single Game Server Implementation!!!\n");
   printf("Illegal action: serverDisconnect.\n");
   exit(EXIT_FAILURE); 
}

void GameServer::processServerPacket(pack::Packet p, int fromid) 
{
   printf("Single Game Server Implementation!!!\n");
   printf("Illegal action: processServerPacket.\n");
   exit(EXIT_FAILURE); 
}

void GameServer::clientSendPacket(Packet &p, int id)
{
   cm.clientSendPacket(p, id);
}

void GameServer::clientBroadcast(Packet &p)
{
   cm.clientBroadcast(p);
}

void GameServer::serverSendPacket(Packet &p, int id)
{
   return; //do nothing
}

void GameServer::serverBroadcast(Packet &p)
{
   return;  //do nothing
}

void GameServer::removeClientConnection(int id)
{
   cm.removeClientConnection(id);
}