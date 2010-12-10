#ifndef _SERVERCONNMANAGER_H_
#define _SERVERCONNMANAGER_H_

#include "packet.h"
#include <map>


void sleepms(int ms);
int currentTicks();
int newId();
void setId(int v);

namespace ServerOps { enum {
   request = 1, anounce=2, good=3, bad=4, ready=5
};}


struct ConnectionInfo {
   ConnectionInfo(int id, sock::Connection conn) : id(id), conn(conn) {}
   sock::Connection conn;
   int id;
};

struct ServerConnectionInfo {
   ServerConnectionInfo(sock::Connection conn, int id, int clientPort, int serverPort)
      : id(id), conn(conn), clientPort(clientPort), serverPort(serverPort) {}
   sock::Connection conn;
   int id, serverPort, clientPort;
};

struct ConnectionManager {
   ConnectionManager() : ownServerId(0),nextServId(0) {}
   void clientSendPacket(pack::Packet p, int id);
   void clientBroadcast(pack::Packet p);
   void serverSendPacket(pack::Packet p, int id);
   void serverBroadcast(pack::Packet p);
 
   void addClientConnection(sock::Connection conn, int id);
   void removeClientConnection(int id);
   void removeClientAt(int index);

   void addServerConnection(sock::Connection conn, int id, int clientPort, int serverPort);
   void removeServerConnection(int id);  
   void removeServerAt(int index);

   void sendServerList(sock::Connection conn);
   bool readServerList(sock::Connection conn, int ownId, int ownCp, int ownSp);

   std::map<int, int> idToClientIndex; 
   std::vector<ConnectionInfo> clientConnections;

   std::map<int, int> idToServerIndex;
   std::vector<ServerConnectionInfo> serverConnections;

   void printPackStat();
   void initPackStat();
   void updatePackStatC(int packType);
   void updatePackStatS(int packType);

   int ownServerId, nextServId;
   std::vector<int> packStatC; //server->client stats
   std::vector<int> packStatS; //server->server stats
};


#endif
