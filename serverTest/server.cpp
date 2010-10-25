#include "socket.h"
#include "packet.h"
#include <cstdio>

using namespace sock;
using namespace std;

int main()
{  
   setupSockets();
   Server serv(27015);
   serv.listen(5);
   vector<Connection> connections;
   
   printf("Accepting connections on port %d\n", serv.port());
   
   while (1) {
      while (serv.select()) {
         printf("Accepting a new connection\n");
         connections.push_back(serv.accept());
         printf("Got it.\n");
      }
   
      for (unsigned i = 0; i < connections.size(); i++) {
         while (connections[i].select()) {
            if (connections[i]) {
               Packet p;
               connections[i].recv(p, 4);
               int val;
               p.readInt(val);
               printf("%d: %d\n", i, val);
               if (val == '+')
                  goto done;
            } else {
               printf("%d disconnected\n", i);
               if (connections.size() > 1) {
                  printf("swapping %d to %d\n", connections.size()-1, i);
                  connections[i] = connections.back();
               }
               connections.pop_back();
               break;
            }
         }
      }
      
      usleep(30000);
   }

done:
   for (unsigned i = 0; i < connections.size(); i++) {
      connections[i].close();
   }

   serv.close();
   
   return 0;
}
