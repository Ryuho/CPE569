#include <iostream>
#include <string>
#include "socket.h"
#include "packet.h"

using namespace std;
using namespace sock;
using namespace packet;

int main (int argc, char *argv[])
{
   int strCount;
   char buffer[256];
   Packet p;
   string line;
   const char *host = "localhost";
   if (argc == 2)
      host = argv[1];


   info test;
   test.playerID = 1;
   test.x = 90.230;
   test.y = 9.03;
   
   setupSockets();

   cout << "connecting..." << endl;
   Connection conn(host, 27019);
   if (conn.check())
      cout << "connected." << endl;
   else
      cout << "failed." << endl;

   do {
      cout << "enter the playerID: ";
      getline(cin,line);
      test.playerID = atoi(line.c_str());
      cout << "enter the x coordinate: ";
      getline(cin,line);
      test.x = atof(line.c_str());
      cout << "enter the y coordinate: ";
      getline(cin,line);
      test.y = atof(line.c_str());

      p.reset().writeInfo(test);
      
      if (conn.send(p.setCursor(0))) {
         if (conn.recv(p)) {
            p.readCStr(buffer);
            cout << "return message: " << buffer << endl;
         }
      }
   } while (cin && conn);

   cout << "closing..." << endl;
   conn.close();
   cout << "closed." << endl;

   shutdownSockets();

   return 0;
}
