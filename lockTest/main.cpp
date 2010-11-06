#include <cstdio>
#include <windows.h>
#include "LockManager.h"
#include <boost/thread.hpp>


void otherThread(LockManager *lm)
{
   lm->acquire(6);

   lm->release(6);
}

int main()
{
   LockManager lm;

   lm.startThread();

   lm.addLocalLock(5);
   lm.addRemoteLock(6, 1);

   boost::thread t(otherThread, &lm);

   Sleep(1000);
   printf("done\n");

   return 0;
}