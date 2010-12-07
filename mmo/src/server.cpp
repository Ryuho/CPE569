#include "GameServer.h"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include "Packets.h"

#ifdef WIN32
#include <Windows.h>
#else
#include <time.h>
#endif

using namespace std;
using sock::Connection;
using sock::Server;
using sock::Packet;
using sock::setupSockets;

int nextId = 100;
int nextServId = 1;

int newId()
{
   return nextId++;
}

void setIdOffset(int sid)
{
   nextId = sid << 24;
}

int newServId()
{
   return nextServId++;
}

ConnectionManager *displayCM;

void displayBandwidth()
{
   static int lastDisplay = 0;
   const static int refreshTime = 5000;
   int ticks = currentTicks();

   if (ticks - lastDisplay > refreshTime) {
      lastDisplay = ticks;
      float skb = sock::getBytesSent() / 1000.0f;
      float rkb = sock::getBytesRead() / 1000.0f;

      printf("s=%2.2fkb|r=%2.2fkb ", skb, rkb);
      displayCM->printPackStat();
   }
}

bool updateServId(int last)
{
   bool ret = last >= nextServId;
   nextServId = max(nextServId, last+1);
   return ret;
}

#define dotest(x) \
   if (true) { \
      x p = x(); \
      printf("%s size: %d\n", #x, p.makePacket().data.size()); \
   }

void testPackets()
{
   /*pack::Arrow ar = pack::Arrow();
   printf("Arrow size: %d\n", ar.makePacket().data.size());
   pack::Click cl = pack::Click()
   printf("Click size: %d\n", cl.makePacket().data.size());
   pack::Connect co = pack::Connect()
   printf("Connect size: %d\n", co.makePacket().data.size());*/

   dotest(pack::Arrow);
   dotest(pack::Click);
   dotest(pack::Connect);
   pack::HealthChange hc = pack::HealthChange(1, 1);
   printf("HealthChange size: %d\n", hc.makePacket().data.size());
   dotest(pack::Initialize);
   dotest(pack::Position);
   dotest(pack::Pvp);
   dotest(pack::Signal);
}

struct TConn {
   TConn(sock::Connection conn) : conn(conn), servId(-1) {}
   int servId, servPort, clientPort;
   Connection conn;
};

int main(int argc, const char* argv[])
{
   printf("Something\n");
   //decalre vars that are going to be used
   int clientPort, serverPort;
   const char* altAddress = 0;
   int altPort = -1;
   int remoteId = -1, remoteClientPort;
   vector<TConn> tconns;

   testPackets();

   if(argc == 3) {
      clientPort = atoi(argv[1]);
      serverPort = atoi(argv[2]);
   } else if (argc == 5) {
      clientPort = atoi(argv[1]);
      serverPort = atoi(argv[2]);
      altAddress = argv[3];
      altPort = atoi(argv[4]);
   } else {
      printf("Usage: server <client port number> <server port number> [ <alternate server address> <alternate server port> ]\r\n");
      printf("Using defaults...\n");
      clientPort = 27027;
      serverPort = 27028;
   }

   setupSockets();

   printf("Starting server with ports: %d and %d\n", clientPort, serverPort);
   Server clientServ(clientPort);
   clientServ.listen(5);
   Server serverServ(serverPort);
   serverServ.listen(5);

   if (!clientServ || !serverServ) {
      printf("Failed to set up sockets on one or more ports\n");
      return -1;
   }

   ConnectionManager cm;
   cm.initPackStat();
   displayCM = &cm;


   if(altPort > 0) {
      printf("Connecting to another server: %s %d\n", altAddress, altPort);
      Connection servConn(altAddress,altPort);
      sock::Packet p;

      if (!servConn) {
         printf("Failed to connect to alternate server\n");
         return -1;
      }
      servConn.send(sock::Packet().writeInt(ServerOps::request).writeInt(clientPort).writeInt(serverPort));
      if (!servConn.select(1000) || !servConn.recv(p, 12)) {
         printf("timed out waiting for host server\n");
         return -1;
      }
      p.readInt(cm.ownServerId).readInt(remoteId).readInt(remoteClientPort);
      printf("Connected to host server. using id %d!\n", cm.ownServerId);
      cm.addServerConnection(servConn, remoteId, remoteClientPort, altPort);

      if (cm.readServerList(servConn, cm.ownServerId, clientPort, serverPort)) {
         printf("Connected to all other servers successfully, sending ready packets\n");

         for (unsigned i = 0; i < cm.serverConnections.size(); i++) {
            cm.serverConnections[i].conn.send(sock::Packet().writeInt(ServerOps::ready));
         }
      } else {
         printf("Unable to connect to all servers, aborting\n");
         return -1;
      }
   } else {
      cm.ownServerId = newServId();
      printf("Server started as independent host. id: %d\n", cm.ownServerId);
   }

   GameServer gs(cm, remoteId); // should also tell game server which server it connected to originally or none at all if independent

   printf("Game server started, accepting client Connections on port %d\n", clientServ.port());

   while (true) {
      displayBandwidth();
      int startTicks = currentTicks();

      while (clientServ.select()) {
         int id = newId();
         cm.addClientConnection(clientServ.accept(), id);
         gs.newClientConnection(id);
      }

      while (serverServ.select()) {
         int id = newId();
         tconns.push_back(TConn(serverServ.accept()));
      }
   
      for (unsigned i = 0; i < cm.clientConnections.size(); i++) {
         Connection conn = cm.clientConnections[i].conn;
         while (conn.select()) {
            if (conn) {
               gs.processClientPacket(pack::readPacket(conn), cm.clientConnections[i].id);
            } else {
               int id = cm.clientConnections[i].id;
               cm.removeClientAt(i--);
               gs.clientDisconnect(id);
               break;
            }
         }
      }

      for (unsigned i = 0; i < cm.serverConnections.size(); i++) {
         Connection conn = cm.serverConnections[i].conn;
         while (conn.select()) {
            if (conn) {
               gs.processServerPacket(pack::readPacket(conn), cm.serverConnections[i].id);
            } else {
               int id = cm.serverConnections[i].id;
               cm.removeServerAt(i--);
               gs.serverDisconnect(id);
               break;
            }
         }
      }

      
      for (unsigned i = 0; i < tconns.size(); i++) {
         Connection conn = tconns[i].conn;
         while (conn.select()) {
            if (conn) {
               sock::Packet p;
               int op;
               if (tconns[i].servId == -1) {
                  conn.recv(p, 4);
                  p.readInt(op);
                  if (op == ServerOps::request) {
                     // request would also tell us about its ports
                     conn.recv(p, 8);
                     int cp, sp;
                     p.readInt(cp).readInt(sp);
                     int sid = newServId();
                     tconns[i].servId = sid;
                     tconns[i].clientPort = cp;
                     tconns[i].servPort = sp;
                     conn.send(p.reset().writeInt(sid).writeInt(cm.ownServerId).writeInt(clientPort)); // Also send server list
                     cm.sendServerList(conn);
                  } else if (op == ServerOps::anounce) {
                     // server will tell us its id and ports, we respond with good or bad
                     conn.recv(p, 12);
                     int sid, cp, sp;
                     p.readInt(sid).readInt(cp).readInt(sp);
                     tconns[i].servId = sid;
                     tconns[i].clientPort = cp;
                     tconns[i].servPort = sp;
                     conn.send(p.reset().writeInt(updateServId(sid) ? ServerOps::good : ServerOps::bad));
                  } else {
                     printf("Got a packet from tconn with unknown id with unknown op: %d\n", op);
                  }
               } else { // We know about this server, waiting for it to tell us it connected to everyone
                  conn.recv(p, 4);
                  p.readInt(op);
                  if (op == ServerOps::ready) {
                     // Server succeeded, add to server lists
                     cm.addServerConnection(conn, tconns[i].servId, tconns[i].clientPort, tconns[i].servPort);
                     gs.newServerConnection(tconns[i].servId);
                     tconns[i--] = tconns.back();
                     tconns.pop_back();
                     break;
                  } else {
                     printf("Got a packet from tconn with id %d and op %d when expecting ready\n", tconns[i].servId, op);
                  }
               }
               //gs.processServerPacket(pack::readPacket(conn), cm.serverConnections[i].id);
            } else {
               conn.close();
               tconns[i--] = tconns.back();
               tconns.pop_back();
               break;
            }
         }
      }

      gs.update(currentTicks());
      
      int sleepTime = 100 - (currentTicks() - startTicks);
      if (sleepTime > 0)
         sleepms(50); // 50ms is pretty long...
      else
         printf("&&& Didn't sleep at all, sleeptime: %d\n", sleepTime);
   }

   for (unsigned i = 0; i < cm.clientConnections.size(); i++) {
      cm.clientConnections[i].conn.close();
   }

   clientServ.close();
   
   return 0;
}




