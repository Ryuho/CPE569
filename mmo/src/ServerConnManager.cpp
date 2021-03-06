#include "ServerConnManager.h"
#include "socket.h"
#include "Constants.h"
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
   std::map<int,int>::iterator iter = idToClientIndex.find(toid);
   if(iter != idToClientIndex.end()) {
      p.sendTo(clientConnections[(*iter).second].conn);
      updatePackStatC(p.type);
   }
   else
	   return;
	   //printf("Error: clientSendPacket: Unable to send to %d\n", toid);
}


void ConnectionManager::clientBroadcast(pack::Packet p)
{
   for (unsigned i = 0; i < clientConnections.size(); i++)
   {
      p.sendTo(clientConnections[i].conn);   
      updatePackStatC(p.type);
   }
}

void ConnectionManager::serverSendPacket(pack::Packet p, int toid)
{
   std::map<int,int>::iterator iter = idToServerIndex.find(toid);
   if(iter != idToServerIndex.end()) {
      p.sendTo(serverConnections[(*iter).second].conn);
	  updatePackStatS(p.type);
   }
   else
      printf("Error: serverSendPacket: Unable to send to %d\n", toid);
}


void ConnectionManager::serverBroadcast(pack::Packet p)
{
   for(unsigned i = 0; i < serverConnections.size(); i++)
      p.sendTo(serverConnections[i].conn);
      updatePackStatS(p.type);
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
   //printf("add client connection\n");
}

void ConnectionManager::removeClientConnection(int id)
{
	printf("removing client id: %d\n",id);
	std::map<int,int>::iterator iter = idToClientIndex.find(id);
   if (iter != idToClientIndex.end()) {
	   removeClientAt(iter->second);
	   //removeClientAt(idToClientIndex[(*iter).second]);
   }
   else
      printf("Error: removeClientConnection: Unable to remove %d\n", id);
}

void ConnectionManager::removeClientAt(int i)
{
	if (clientConnections[i].conn)
      clientConnections[i].conn.close();
   //else
	//   printf("Tried to remove connection at %d that had no connection \n",i);

   int cID = clientConnections[i].id;
   if (clientConnections.size() > 1) {
      clientConnections[i] = clientConnections.back();
      idToClientIndex[clientConnections[i].id] = i;
   }
   idToClientIndex.erase(cID);
   clientConnections.pop_back();
}

// server -> server connection functions
void ConnectionManager::addServerConnection(Connection conn, int id, int clientPort, int serverPort)
{
   map<int,int>::iterator itr = idToServerIndex.find(id);
   if (itr != idToServerIndex.end()) {
      printf("Error, duplicate server id: %d\n", id);
      exit(-1);
   }

   idToServerIndex[id] = serverConnections.size();
   serverConnections.push_back(ServerConnectionInfo(conn, id, clientPort, serverPort));
}

void ConnectionManager::removeServerConnection(int id)
{
   std::map<int,int>::iterator iter = idToServerIndex.find(id);
   if (iter != idToServerIndex.end()) {
	   removeServerAt(iter->second);
   }
   else
      printf("Error: removeServerConnection: Unable to remove %d\n", id);
}

void ConnectionManager::removeServerAt(int i)
{
   if (serverConnections[i].conn)
      serverConnections[i].conn.close();

   int sID = serverConnections[i].id;
   if (serverConnections.size() > 1) {
      serverConnections[i] = serverConnections.back();
      idToServerIndex[serverConnections[i].id] = i;
   }
   idToServerIndex.erase(sID);
   serverConnections.pop_back();
}

void ConnectionManager::sendServerList(sock::Connection conn)
{
   Packet p;
   p.writeInt(serverConnections.size());

   for (unsigned i = 0; i < serverConnections.size(); i++) {
      p.writeLong(serverConnections[i].conn.getAddr())
         .writeInt(serverConnections[i].id)
         .writeInt(serverConnections[i].clientPort)
         .writeInt(serverConnections[i].serverPort);
   }

   conn.send(p);
}

bool ConnectionManager::readServerList(sock::Connection conn, int ownId, int ownCp, int ownSp)
{
   Packet p, p2;
   int n;

   if (!conn.recv(p, 4))
      return false;
   p.readInt(n);

   if (!conn.recv(p, n*(sizeof(unsigned long) + 3*sizeof(int))))
      return false;

   unsigned long addr;
   int id, cp, sp;
   for (int i = 0; i < n; i++) {
      p.readLong(addr).readInt(id).readInt(cp).readInt(sp);
      Connection serv(addr, sp);

      if (!serv)
         return false;

      if (!serv.send(p2.reset().writeInt(ServerOps::anounce).writeInt(ownId).writeInt(ownCp).writeInt(ownSp)))
         return false;

      if (!serv.select(1000) || !serv.recv(p2, 4))
         return false;

      int op;
      p2.readInt(op);
      if (op != ServerOps::good)
         return false;

      addServerConnection(serv, id, cp, sp);
   }

   return true;
}

void ConnectionManager::printPackStat()
{
   printf("Server-Server ");
   for(unsigned i = 1; i < packStatC.size(); i++) {
      printf(" %d", packStatC[i]);
   }
   printf("\n");
   printf("Server-Client ");
   for(unsigned i = 1; i < packStatS.size(); i++){
      printf(" %d", packStatS[i]);
   }
   printf("\n\n");
   initPackStat();
}

void ConnectionManager::initPackStat()
{
   packStatC.clear();
   packStatS.clear();
   for(unsigned i = 0; i < constants::PacketType::MaxPacketType; i++){
      packStatC.push_back(0);
      packStatS.push_back(0);
   }
   
}

void ConnectionManager::updatePackStatC(int packType)
{
   if(packType >= 0 && packType < constants::PacketType::MaxPacketType)
      packStatC[packType]++;
}

void ConnectionManager::updatePackStatS(int packType)
{
   if(packType >= 0 && packType < constants::PacketType::MaxPacketType)
      packStatS[packType]++;
}