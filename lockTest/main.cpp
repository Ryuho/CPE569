#include <cstdio>
#include "LockManager.h"
#include <boost/thread.hpp>
//#include <unistd.h>
#include <windows.h>
#include "socket.h"

using namespace sock;

void sleep(int ms)
{
   Sleep(ms);//usleep(ms*1000);
}

void otherThread(int t)
{
   LockManager lm;

   lm.startThread(27030 + t, t);
   lm.addHost("localhost", 27030, 0);

   lm.addLocalLock(5+t);
   lm.addRemoteLock(5, 0);

   for(int i = 0; i < 15; i++) {
      //printf("ta%d\n", t);
      lm.acquire(5);
      printf("t%d a\n", t);
      sleep(1000);
      printf("t%d r\n", t);
      lm.release(5);
      sleep(1000 + rand()%500);
   }

   printf("tdone\n");

   /*Connection c("localhost", 27030);
   
   if (c) {
      Packet p;

      for (int i = 0; i < 1000; i++) {
         c.send(p.reset().writeInt(ops::acquire).writeInt(5));
         //printf("t%d: recv\n", t);
         c.recv(p, 4);
         sleep(1);
         //printf("trelease\n");
         c.send(p.reset().writeInt(ops::release).writeInt(5));
         printf("t%d: %d\n", t, i);
      }
      printf("t%d done\n", t);
   } else {
      printf("Couldn't connect!\n");
   }*/
}

int main()
{
   LockManager lm;

   lm.startThread(27030, 0);

   lm.addLocalLock(5);
   lm.addRemoteLock(6, 1);

   //lm.addHost("stuff", 6969, 28);
   
   //lm.addLocalLock(5);

   boost::thread t(otherThread, 1);
   boost::thread t2(otherThread, 2);
   
   /*for (int i = 0; i < 1000; i++) {
      lm.acquire(5);
      sleep(1);
      //printf("mrelease\n");
      lm.release(5);
      printf("m: %d\n", i);
   }*/

   
   sleep(50);

   for(int i = 0; i < 500; i++) {
      //printf("ma\n");
      lm.acquire(5);
      printf("m%d a\n", i);
      sleep(5);
      printf("m%d r\n", i);
      lm.release(5);
   }
   
   //sleep(1000);
   printf("mdone\n");

   t.join();
   t2.join();

   return 0;
}
