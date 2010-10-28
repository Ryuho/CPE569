#include "GameServer.h"
#include <cstdio>

#ifdef WIN32
#include <Windows.h>
#endif

using namespace std;
using sock::Connection;
using sock::Server;
using sock::Packet;
using sock::setupSockets;

void sleepms(int ms)
{
#ifdef WIN32
   Sleep(ms);
#else
   usleep(ms*1000);
#endif
}

int newId()
{
   static int id = 100;
   return id++;
}

int main()
{  
   setupSockets();
   Server serv(27027);
   serv.listen(5);

   ConnectionManager cm;
   GameServer gs(cm);

   printf("Accepting connections on port %d\n", serv.port());
   
   while (serv) {
      while (serv.select()) {
         int id = newId();
         cm.addConnection(serv.accept(), id);
         gs.newConnection(id);
      }
   
      for (unsigned i = 0; i < cm.connections.size(); i++) {
         Connection conn = cm.connections[i].conn;
         while (conn.select()) {
            if (conn) {
               gs.processPacket(pack::readPacket(conn), cm.connections[i].id);
            } else {
               int id = cm.connections[i].id;
               cm.removeAt(i--);
               gs.disconnect(id);
               break;
            }
         }
      }
      
      sleepms(3); // 3 ms delay, really fast...
   }

   for (unsigned i = 0; i < cm.connections.size(); i++) {
      cm.connections[i].conn.close();
   }

   serv.close();
   
   return 0;
}

void ConnectionManager::sendPacket(pack::Packet p, int toid)
{
   p.sendTo(connections[idToIndex[toid]].conn);
}

void ConnectionManager::addConnection(Connection conn, int id)
{
   map<int,int>::iterator itr = idToIndex.find(id);
   if (itr != idToIndex.end()) {
      printf("Error, duplicate connection id: %d\n", id);
      exit(-1);
   }

   idToIndex[id] = connections.size();
   connections.push_back(ConnectionInfo(id, conn));
}

void ConnectionManager::removeConnection(int id)
{
   removeAt(idToIndex[id]);
}

void ConnectionManager::removeAt(int i)
{
   if (connections[i].conn)
      connections[i].conn.close();

   idToIndex.erase(connections[i].id);
   if (connections.size() > 1) {
      connections[i] = connections.back();
      idToIndex[connections[i].id] = i;
   }
   connections.pop_back();
}

void ConnectionManager::broadcast(pack::Packet p)
{
   for (unsigned i = 0; i < connections.size(); i++)
      p.sendTo(connections[i].conn);
}