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

   int port;
   const char* address;

   if(argc != 3){
      printf("Usage: client <server address> <port number>\n");
      address = "localhost";
      port = 27027;
   }
   else{
      address = argv[1];
      port = atoi(argv[2]);
   }

   while(1){
      srand((unsigned)time(0));
      app.init(address,port);

      while (app.alive) {
         app.update();
      }
   }

   return 0;
}
