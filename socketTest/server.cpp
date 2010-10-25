#include <iostream>
#include <string>
#include "socket.h"
#include "packet.h"

using namespace std;
using namespace sock;
using namespace packet;

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

      info temp;

      int numStr;
      bool dontexit = true;
      while (conn && dontexit) {
         if (conn.recv(p)) {
            p.readInfo(temp);
            if(temp.playerID == 1){
               p.reset().writeCStr("Player ID accepted!");
               cout << "Player 1's (x,y) = (" << 
                  temp.x << "," << temp.y << ")" << endl;
            }
            else{
               p.reset().writeCStr("Player ID not recognized!");

            }
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
