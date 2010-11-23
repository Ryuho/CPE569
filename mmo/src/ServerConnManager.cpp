#include "ServerConnManager.h"
#include "socket.h"
#include <stdio.h>

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

void ConnectionManager::clientSendPacket(pack::Packet p, int toid)
{
   p.sendTo(clientConnections[idToClientIndex[toid]].conn);
}


void ConnectionManager::clientBroadcast(pack::Packet p)
{
   for (unsigned i = 0; i < clientConnections.size(); i++)
      p.sendTo(clientConnections[i].conn);
}

void ConnectionManager::serverSendPacket(pack::Packet p, int toid)
{
   p.sendTo(serverConnections[idToServerIndex[toid]].conn);
}


void ConnectionManager::serverBroadcast(pack::Packet p)
{
   for (unsigned i = 0; i < serverConnections.size(); i++)
      p.sendTo(serverConnections[i].conn);
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
   map<int,int>::iterator itr = idToServerIndex.find(id);
   if (itr != idToServerIndex.end()) {
      printf("Error, duplicate server id: %d\n", id);
      exit(-1);
   }

   idToServerIndex[id] = serverConnections.size();
   serverConnections.push_back(ConnectionInfo(id, conn));
}

void ConnectionManager::removeServerConnection(int id)
{
   removeServerAt(idToServerIndex[id]);
}

void ConnectionManager::removeServerAt(int i)
{
   if (serverConnections[i].conn)
      serverConnections[i].conn.close();

   idToServerIndex.erase(serverConnections[i].id);
   if (serverConnections.size() > 1) {
      serverConnections[i] = serverConnections.back();
      idToServerIndex[serverConnections[i].id] = i;
   }
   serverConnections.pop_back();
}
