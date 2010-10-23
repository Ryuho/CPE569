#include <iostream>
#include <string>
#include "socket.h"

using namespace std;
using namespace sock;

int main (int argc, char *argv[])
{
   int strCount;
   char buffer[256];
   Packet p;
   string line;
   const char *host = "localhost";
   if (argc == 2)
      host = argv[1];

   setupSockets();

   cout << "connecting..." << endl;
   Connection conn(host, 27019);
   if (conn.check())
      cout << "connected." << endl;
   else
      cout << "failed." << endl;

   do {
      cout << "enter strings ending with a blank line: " << endl;
      strCount = 0;
      p.reset().writeInt(0);
      while (getline(cin, line) && line != "") {
         p.writeStdStr(line);
         strCount++;
      }
      if (strCount > 0 && conn.send(p.setCursor(0).writeInt(strCount))) {
         if (conn.recv(p)) {
            p.readCStr(buffer);
            cout << "buffer: " << buffer << endl;
         }
      }
   } while (cin && conn);

   cout << "closing..." << endl;
   conn.close();
   cout << "closed." << endl;

   shutdownSockets();

   return 0;
}
