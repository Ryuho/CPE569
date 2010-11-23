#ifndef _SERVERCONNMANAGER_H_
#define _SERVERCONNMANAGER_H_

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
   void clientSendPacket(pack::Packet p, int id);
   void clientBroadcast(pack::Packet p);
   void serverSendPacket(pack::Packet p, int id);
   void serverBroadcast(pack::Packet p);

   template<typename T>
   void clientSendPacket(T t, int id) {
      clientSendPacket(t.makePacket(), id);
   }
   template<typename T>
   void clientBroadcast(T t) {
      clientBroadcast(t.makePacket());
   }

   template<typename T>
   void serverSendPacket(T t, int id) {
      serverSendPacket(t.makePacket(), id);
   }
   template<typename T>
   void serverBroadcast(T t) {
      serverBroadcast(t.makePacket());
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