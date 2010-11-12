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
   };
}


struct LockManager {
   LockManager() {}
   
   void startThread(int port, int id);
   void addHost(const char *hostname, int port, int id);

   void addLocalLock(int id);
   void addRemoteLock(int id, int host);

   void acquire(int id);
   void release(int id);

   void deleteLock(int id);

   LMData *data;
};

#endif
