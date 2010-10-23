#include <iostream>
#include <string>
#include "socket.h"

using namespace std;
using namespace sock;

int main ()
{
   setupSockets();

   Server serv(27019);
   serv.listen(5);

   cout << "ready to accept connections." << endl;

   while (1) {
      Connection conn = serv.accept();
      cout << "accepted a connection." << endl;
      string line;
      Packet p;
      int numStr;
      bool dontexit = true;
      while (conn && dontexit) {
         if (conn.recv(p)) {
            p.readInt(numStr);
            for (int i = 0; i < numStr; i++) {
               p.readStdStr(line);
               cout << " -- " << line << " -- " << endl;
               if (line == "exit")
                  dontexit = false;
            }
            p.reset().writeCStr("owned");
            if (dontexit && conn.send(p)) {
               cout << "response sent." << endl;
            }
         }
         /*
         if (conn.recv(p, 50)) {
            p.readStdStr(line);
            cout << "got: " << line << endl;
            conn.send(p.reset(50).writeStdStr(line + " sucks"));
         }*/
      }
      cout << "closing the connection." << endl;
      conn.close();
      cout << "connection closed." << endl;
   }

   shutdownSockets();

   return 0;
}
