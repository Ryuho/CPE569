#include <cstdio>
#include "LockManager.h"
#include <boost/thread.hpp>
//#include <unistd.h>
#include <windows.h>
#include "socket.h"

using namespace sock;


struct DataTest : public LockData {
   DataTest() {
      vals[0] = 0;
      vals[1] = 0;
      vals[2] = 0;
   }

   virtual void sendLockData(int id, sock::Connection c) {
      c.send(Packet().writeInt(vals[id-5]));
   }

   virtual bool recvLockData(int id, sock::Connection c) {
      Packet p;
      if (!c.recv(p, 4))
         return false;
      p.readInt(vals[id-5]);
      return true;
   }

   int vals[3];
};

void sleep(int ms)
{
   Sleep(ms);//usleep(ms*1000);
}

void otherThread(int t)
{
   LockManager lm;
   DataTest *dt = new DataTest;

   lm.startThread(27030 + t, t, dt);
   lm.addHost("localhost", 27030, 0);

   lm.addRemoteLock(5, 0);
   if (t == 1) {
      lm.addLocalLock(6);
      lm.addHost("localhost", 27032, 2);
      lm.addRemoteLock(7, 2);
      dt->vals[1] = 20000;
	  //lm.shutDown();
   } else {
      lm.addLocalLock(7);
      lm.addHost("localhost", 27031, 1);
      lm.addRemoteLock(6, 1);
      dt->vals[2] = 30000;
   }

   //if(t != 1) {
   for(int i = 0; i < 500; i++) {
      int id = 5 + rand()%3;
      lm.acquire(id);
      
	  //printf("t%d a\n", t);
      dt->vals[id-5]++;
      //printf("t%d r\n", t);
      lm.release(id); 
   }

   printf("tdone-%d\n",t);
   lm.acquire(5);
   lm.acquire(6);
   lm.acquire(7);
   lm.release(7);
   lm.release(6);
   lm.release(5);
   printf("0: %d, 1: %d, 2: %d\n", dt->vals[0], dt->vals[1], dt->vals[2]);
   //}

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
   DataTest dt;

   dt.vals[0] = 10000;

   lm.startThread(27030, 0, &dt);

   lm.addLocalLock(5);
   lm.addRemoteLock(6, 1);
   lm.addRemoteLock(7, 2);

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
      int id = 5 + rand()%3;
      lm.acquire(id);
      //printf("m%d a\n", i);
      dt.vals[id-5]++;
      //printf("m%d r\n", i);
      lm.release(id);
   }

   if(lm.acquire(999999))
   {
	   printf("acquired lock for id 999999 that should not exist\n");
   }
   lm.release(99999999);
   
   
   //sleep(1000);
   printf("mdone\n");
   printf("0: %d, 1: %d, 2: %d\n", dt.vals[0], dt.vals[1], dt.vals[2]);

   t.join();
   t2.join();

   lm.acquire(5);
   lm.acquire(6);
   lm.acquire(7);
   lm.release(7);
   lm.release(6);
   lm.release(5);
   
   printf("0: %d, 1: %d, 2: %d\n", dt.vals[0], dt.vals[1], dt.vals[2]);
   printf("total: %d\n", dt.vals[0] + dt.vals[1] + dt.vals[2] - 60000);

   return 0;
}
