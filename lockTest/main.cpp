#include <cstdio>
#include "LockManager.h"
#include <boost/thread.hpp>
#include <unistd.h>
#include "socket.h"

using namespace sock;

void sleep(int ms)
{
   usleep(ms*1000);
}

void otherThread()
{
   Connection c("localhost", 27030);
   
   if (c) {
      Packet p;
      printf("sending acquire\n");
      c.send(p.writeInt(ops::acquire).writeInt(5));
      printf("waiting for reply\n");
      c.recv(p);
      sleep(2000);
      c.send(p.reset().writeInt(ops::release).writeInt(5));
      printf("finished\n");
   } else {
      printf("Couldn't connect!\n");
   }
}

int main()
{
   LockManager lm;

   lm.startThread();
   
   lm.addLocalLock(5);

   boost::thread t(otherThread);

   sleep(1000);
   
   lm.acquire(5);
   
   printf("done\n");

   return 0;
}
