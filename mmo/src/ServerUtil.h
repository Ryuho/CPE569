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

   void addConnection(sock::Connection conn, int id);
   void removeConnection(int id);
   void removeAt(int index);
   std::map<int, int> idToIndex;
   std::vector<ConnectionInfo> connections;
};

#endif