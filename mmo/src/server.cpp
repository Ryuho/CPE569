#include "socket.h"
#include "packet.h"
#include <cstdio>

#ifdef WIN32
#include <Windows.h>
#endif

using namespace std;
using sock::Connection;
using sock::Server;
using sock::Packet;
using sock::setupSockets;

void sleepms(int ms)
{
#ifdef WIN32
   Sleep(ms);
#else
   usleep(ms*1000);
#endif
}

struct ConnectionInfo {
   ConnectionInfo(int id, Connection conn) : id(id), conn(conn) {}
   Connection conn;
   int id;
};

int newId()
{
   static int id = 100;
   return id++;
}

int main()
{  
   setupSockets();
   Server serv(27027);
   serv.listen(5);
   vector<ConnectionInfo> connections;
   
   printf("Accepting connections on port %d\n", serv.port());
   
   while (serv) {
      while (serv.select()) {
         connections.push_back(ConnectionInfo(newId(), serv.accept()));
         printf("New connection. id: %d\n", connections.back().id);
         pack::Connect(connections.back().id).makePacket().sendTo(connections.back().conn);
      }
   
      for (unsigned i = 0; i < connections.size(); i++) {
         Connection c = connections[i].conn;
         while (c.select()) {
            if (c) {
               pack::Packet p = pack::readPacket(c);
               if (p.type == pack::pos) {
                  pack::Pos pos(p);
                  pos.id = connections[i].id;
                  pack::Packet p2 = pos.makePacket();
                  for (unsigned j = 0; j < connections.size(); j++) {
                     p2.sendTo(connections[j].conn);
                  }
               }
            } else {
               printf("%d disconnected\n", connections[i].id);
               pack::Signal sig(pack::Signal::disconnect, connections[i].id);
               pack::Packet p = sig.makePacket();
               if (i != connections.size() - 1) {
                  printf("swapping %d to %d\n", connections.size()-1, i);
                  connections[i] = connections.back();
               }
               connections.pop_back();
               for (unsigned j = 0; j < connections.size(); j++) {
                  p.sendTo(connections[j].conn);
               }
               break;
            }
         }
      }
      
      sleepms(3); // 3 ms delay, really fast...
   }

   for (unsigned i = 0; i < connections.size(); i++) {
      connections[i].conn.close();
   }

   serv.close();
   
   return 0;
}
