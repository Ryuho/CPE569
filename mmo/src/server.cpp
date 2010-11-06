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

int main()
{  
   int portNumber = 27027;
   //look at serverrc and see what port to use
   std::ifstream myfile("serverrc");
   if (myfile.is_open())
   {
      std::string data;
      //read in a comment
      std::getline (myfile,data);
      //read in the port number
      std::getline (myfile,data);
      cout << "Port=" << data << endl;
      portNumber = atoi(data.c_str());
      //myfile << "This is another line.\n";
      myfile.close();
   }
   else
   {
      cout << "serverrc file missing!";
   }
  
   setupSockets();
   Server serv(portNumber);
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

      gs.update(currentTicks());
      
      sleepms(50); // 50ms is pretty long...
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
