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

#ifdef WIN32
void sleepms(int ms)
{
   Sleep(ms);
}

int currentTicks()
{
   static int offset = 0;
   LARGE_INTEGER li, freq;
   QueryPerformanceCounter(&li);
   QueryPerformanceFrequency(&freq);
   if (offset == 0)
      offset = (int)(li.QuadPart * 1000 /freq.QuadPart );
   return (int)(li.QuadPart * 1000 /freq.QuadPart ) - offset;
}

#else

void sleepms(int ms)
{
   usleep(ms*1000);
}

int currentTicks()
{
   timespec ts;
   clock_gettime(CLOCK_MONOTONIC, &ts);
   return ts.tv_sec*1000 + ts.tv_nsec/1000000;
}

#endif
int newId()
{
   static int id = 100;
   return id++;
}

int main(int argc, const char* argv[])
{  
   //decalre vars that are going to be used
   int portNumber;
   const char* altAddress;
   int altPort;

   if(argc == 1){
      printf("Usage: server <client port number> <alternate server address> <alternate server port>\n");
      portNumber = 27027;
      altPort = 27028;
   }
   else if(argc >= 2){
      printf("port=|%s|\n",argv[1]);
      portNumber = atoi(argv[1]);
   }
  
   setupSockets();
   Server clientServ(portNumber);
   clientServ.listen(5);

   ConnectionManager cm;

   if(argc == 4){
      altAddress = argv[2];
      altPort = atoi(argv[3]);
      Connection servConn(altAddress,altPort);
      cm.addServerConnection(servConn,newId());
   }

   Server serverServ(altPort);
   serverServ.listen(5);

   GameServer gs(cm);

   printf("Accepting client Connections on port %d\n", clientServ.port());
   
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

      /*for (unsigned i = 0; i < cm.serverConnections.size(); i++) {
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
      }*/

      gs.update(currentTicks());
      
      sleepms(50); // 50ms is pretty long...
   }

   for (unsigned i = 0; i < cm.clientConnections.size(); i++) {
      cm.clientConnections[i].conn.close();
   }

   clientServ.close();
   
   return 0;
}

void ConnectionManager::sendPacket(pack::Packet p, int toid)
{
   p.sendTo(clientConnections[idToClientIndex[toid]].conn);
}


void ConnectionManager::broadcast(pack::Packet p)
{
   for (unsigned i = 0; i < clientConnections.size(); i++)
      p.sendTo(clientConnections[i].conn);
}

// server -> client connection functions

void ConnectionManager::addClientConnection(Connection conn, int id)
{
   map<int,int>::iterator itr = idToClientIndex.find(id);
   if (itr != idToClientIndex.end()) {
      printf("Error, duplicate connection id: %d\n", id);
      exit(-1);
   }

   idToClientIndex[id] = clientConnections.size();
   clientConnections.push_back(ConnectionInfo(id, conn));
}

void ConnectionManager::removeClientConnection(int id)
{
   removeClientAt(idToClientIndex[id]);
}

void ConnectionManager::removeClientAt(int i)
{
   if (clientConnections[i].conn)
      clientConnections[i].conn.close();

   idToClientIndex.erase(clientConnections[i].id);
   if (clientConnections.size() > 1) {
      clientConnections[i] = clientConnections.back();
      idToClientIndex[clientConnections[i].id] = i;
   }
   clientConnections.pop_back();
}

// server -> server connection functions

void ConnectionManager::addServerConnection(Connection conn, int id)
{
   map<int,int>::iterator itr = idToClientIndex.find(id);
   if (itr != idToClientIndex.end()) {
      printf("Error, duplicate connection id: %d\n", id);
      exit(-1);
   }

   idToServerIndex[id] = serverConnections.size();
   serverConnections.push_back(ConnectionInfo(id, conn));
}

void ConnectionManager::removeServerConnection(int id)
{
   removeClientAt(idToClientIndex[id]);
}

void ConnectionManager::removeServerAt(int i)
{
   if (clientConnections[i].conn)
      clientConnections[i].conn.close();

   idToClientIndex.erase(clientConnections[i].id);
   if (clientConnections.size() > 1) {
      clientConnections[i] = clientConnections.back();
      idToClientIndex[clientConnections[i].id] = i;
   }
   clientConnections.pop_back();
}



