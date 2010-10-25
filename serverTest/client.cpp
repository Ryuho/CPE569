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
      Packet p;
      int val;
      
      printf("0\n");
      val = cin.get();
      printf("1\n");
      p.reset().writeInt(val);
      printf("2\n");
      conn.send(p);
      printf("sent %d, conn: %d\n", val, conn.check());
   }
   
   if (conn.check()) {
      conn.close();
   } else
      printf("Other side disconnected\n");

   return 0;
}
