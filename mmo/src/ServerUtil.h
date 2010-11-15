#ifndef _SERVER_UTIL_H_
#define _SERVER_UTIL_H_

#include "packet.h"
#include <map>

void sleepms(int ms);
int currentTicks();
int newId();

struct ConnectionInfo {
   ConnectionInfo(int id, sock::Connection conn) : id(id), conn(conn) {}
   sock::Connection conn;
   int id;
};

struct ConnectionManager {
   void sendPacket(pack::Packet p, int id);
   void broadcast(pack::Packet p);

   template<typename T>
   void sendPacket(T t, int id) {
      sendPacket(t.makePacket(), id);
   }
   template<typename T>
   void broadcast(T t) {
      broadcast(t.makePacket());
   }


   
   void addClientConnection(sock::Connection conn, int id);
   void removeClientConnection(int id);
   void removeClientAt(int index);

   void addServerConnection(sock::Connection conn, int id);
   void removeServerConnection(int id);  
   void removeServerAt(int index);


   std::map<int, int> idToClientIndex; 
   std::vector<ConnectionInfo> clientConnections;

   std::map<int, int> idToServerIndex;
   std::vector<ConnectionInfo> serverConnections;
};

#endif
