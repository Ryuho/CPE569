#include "SDLApp.h"
#include <string>
#include <iostream>
#include <fstream>

using namespace std;

int main (int argc, char *argv[])
{
   SDLApp app;
   
   int port;
   const char* address;

   if(argc != 3){
      printf("Usage: client <server address> <port number>\n");
      exit(0);
   }
   else{
      address = argv[1];
      port = atoi(argv[2]);
   }

   srand((unsigned)time(0));
   app.init(address,port);

   while (1) {
      app.update();
      app.draw();
   }

   return 0;
}

