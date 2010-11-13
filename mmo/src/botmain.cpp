#include <stdio.h>
#include <time.h>
#include <string>
#include <iostream>
#include <fstream>
#include "BotApp.h"

using namespace std;

int main(int argc, char *argv[])
{
   BotApp app;

   //look at clientrc and see what port to use
   int port = 27027;
   string *host = new string("localhost");
   std::ifstream myfile("clientrc");
   if (myfile.is_open())
   {
      string data;
      //read in a comment
      std::getline(myfile,data);
      //read in the port number
      std::getline(myfile,data);
      cout << "server=" << data << endl;
      host = new string(data);
      std::getline (myfile,data);
      port = atoi(data.c_str());
      //myfile << "This is another line.\n";
      myfile.close();
   }
   else
   {
      cout << "clientrc file missing!" << endl;
   }

   srand((unsigned)time(0));
   app.init(host->c_str(), port);

   while (1) {
      app.update();
   }

   return 0;
}