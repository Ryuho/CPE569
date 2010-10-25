#include "socket.h"
#include "packet.h"
#include <cstdio>
#include <iostream>

using namespace sock;
using namespace std;

int main()
{
   Connection conn("localhost", 27015);

   if (!conn) {
      printf("Could not connect\n");
      return 0;
   }

   while (conn.check() && cin) {
      string line;
      getline(cin, line);
      
      pack::Message p(line);
      
      if (!p.makePacket().sendTo(conn))
         printf("Failed sending packet\n");
   }
   
   if (conn.check()) {
      conn.close();
   } else
      printf("Other side disconnected\n");

   return 0;
}
