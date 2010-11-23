#include "GameServer.h"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>

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


int newId()
{
   static int id = 100;
   return id++;
}

int main(int argc, const char* argv[])
{
   printf("Something\n");
   //decalre vars that are going to be used
   int clientPort, serverPort;
   const char* altAddress = 0;
   int altPort = -1;

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

   if(altPort > 0) {
      printf("Connecting to another server: %s %d\n", altAddress, altPort);
      Connection servConn(altAddress,altPort);
      if (!servConn) {
         printf("Failed to connect to alternate server\n");
         return -1;
      }
      cm.addServerConnection(servConn,newId());
   } else {
      printf("Server started as independant host.\n");
   }

   GameServer gs(cm);

   printf("Game server started, accepting client Connections on port %d\n", clientServ.port());

   while (true) {
      while (clientServ.select()) {
         int id = newId();
         cm.addClientConnection(clientServ.accept(), id);
         gs.newClientConnection(id);
      }

      while (serverServ.select()) {
         int id = newId();
         cm.addServerConnection(serverServ.accept(), id);
         gs.newServerConnection(id);
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

      gs.update(currentTicks());
      
      sleepms(50); // 50ms is pretty long...
   }

   for (unsigned i = 0; i < cm.clientConnections.size(); i++) {
      cm.clientConnections[i].conn.close();
   }

   clientServ.close();
   
   return 0;
}




