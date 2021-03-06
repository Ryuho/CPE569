#ifndef _LOCK_MANAGER_H_
#define _LOCK_MANAGER_H_

#include "socket.h"
#include <map>
#include <vector>
#include <boost/thread/mutex.hpp>

struct LMData;

namespace ops {
   enum {
      acquire,
      release,
      success,
      available,
      newHost,
      connect,
      failure,
      shutdown,
   };
}

struct LockData {
   virtual void sendLockData(int id, sock::Connection c) =0;
   virtual bool recvLockData(int id, sock::Connection c) =0;
};

struct LockManager {
   LockManager() {}
   
   void startThread(int port, int id, LockData *lockData = 0);
   void addHost(const char *hostname, int port, int id);

   void addLocalLock(int id);
   void addRemoteLock(int id, int host);

   bool acquire(int id);
   void release(int id);

   void deleteLock(int id);
   void shutDown();

   LMData *data;
};

#endif
