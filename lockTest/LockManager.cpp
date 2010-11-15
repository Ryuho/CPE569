#include "LockManager.h"
#include <boost/thread.hpp>
#include <boost/ref.hpp>
#include <cstdio>
#include <queue>
#include "socket.h"

using namespace sock;
using namespace std;
using namespace boost;

struct Lock {
   Lock(int owner) : owner(owner) {}
   mutex m;
   int owner;
   queue<int> waitList;
};

struct Host {
   Host() : id(-2) {}
   Host(Connection c, int id) : c(c), id(id) {}
   Connection c;
   int id;
};

struct LMData {
   LMData(Server serv) : serv(serv), exitNow(false), localWaitingOn(-1) {}
   void operator()();
   bool lockExists(int id);

   thread t;
   shared_mutex m; // mutex for modifying data structures
   map<int, Lock*> locks;
   Connection self;
   int ownId;
   LockData *lockData;

private: // Thread private storage
   bool hostExists(int socket);
   void handlePacket(Connection c);
   map<int, Host> hosts;
   map<int, int> idToSocket;
   Server serv;
   Connection toSelf;
   SelectSet ss;
   bool exitNow;
   int localWaitingOn;
};

void LockManager::startThread(int port, int id, LockData *lockData)
{
   setupSockets();
   Server serv(port);
   serv.listen(5);
   data = new LMData(serv);

   data->m.lock();
   data->ownId = id;
   data->self = Connection("localhost", serv.port());
   data->lockData = lockData;
   data->t = thread(ref(*data));
   data->m.unlock();
}

void LMData::operator()()
{
   serv.select(-1);
   
   toSelf = serv.accept(); // assuming first connection is local machine

   ss.add(serv.getSocket());
   ss.add(toSelf.getSocket());
   
   // Might want to make a special check like for serv
   // and not add it to hosts. ok for now
   hosts[toSelf.getSocket()] = Host(toSelf, -1);
   idToSocket[-1] = toSelf.getSocket();

   while (1) {
      //printf("%d waiting on select...\n", ownId);
      vector<int> readySds = ss.select(-1);
      //printf("%d got packet\n", ownId);
      
      for (unsigned i = 0; i < readySds.size(); i++) {
         if (readySds[i] == serv.getSocket()) {
            Connection c = serv.accept();
            ss.add(c.getSocket());
            hosts[c.getSocket()] = Host(c, -2);
            printf("%d Got a new lock connection %d\n", ownId, c.getSocket());
         } else {
			Connection c = hosts[readySds[i]].c;
            while (c && c.select()) {
               handlePacket(c);
               if (exitNow)
                  return;
            }
            if (!c) {
               if (c.getSocket() == localWaitingOn) {
                  toSelf.send(Packet().writeInt(ops::failure).writeInt(0));
               }
               ss.remove(c.getSocket());
               idToSocket.erase(idToSocket.find(hosts[c.getSocket()].id));
               hosts.erase(hosts.find(c.getSocket()));
               printf("%d remote host %d disconnected\n", ownId, c.getSocket());

            }
         }
      }
   }

}

void LMData::handlePacket(Connection c)
{
   int op, id;
   Packet p;

   if (!c.recv(p, 8))
      return;
   p.readInt(op).readInt(id);

   /*if (!lockExists(id)) {
      printf("Got request for a lock that does not exist: op: %d, id: %d\n", op, id);
      c.send(Packet().writeInt(ops::failure).writeInt(id));
   }*/

   if (op == ops::acquire) {
      //printf("%d packet Acquire\n", ownId);
      if (!lockExists(id)) {
         printf("Acquire for lock that doesn't exist: %d\n", id);
         c.send(Packet().writeInt(ops::failure).writeInt(id));
         return;
      }
      m.lock_shared();
      //printf("%d Acquire packet locked\n", ownId);
      if (locks[id]->owner == -1) {
         if (locks[id]->m.try_lock()) {
            //printf("%d locked and sending success to %d\n", ownId, hosts[c.getSocket()].id);
            m.unlock_shared();
            c.send(p.reset().writeInt(ops::success).writeInt(id));
            if (lockData) {
               lockData->sendLockData(id, c);
            }
            //printf("acquired %d\n", id);
         } else {
            //printf("%d waitlisted\n", ownId);
            locks[id]->waitList.push(c.getSocket());
            m.unlock_shared();
            //printf("put %d on wait list\n", c.getSocket());
         }
      } else if (c.getSocket() == toSelf.getSocket()) {
         // Forward a request to appropriate lock manager
         //printf("%d forwarding acquire to %d\n", ownId, locks[id]->owner);
         m.unlock_shared();
		 /**if(!hostExists(locks[id]->owner))
		 {
			 toSelf.send(Packet().writeInt(ops::failure).writeInt(id));
			 return;
		 }**/
         hosts[idToSocket[locks[id]->owner]].c.send(p);
         localWaitingOn = hosts[idToSocket[locks[id]->owner]].c.getSocket();
      } else {
         m.unlock_shared();
         printf("Got request for lock %d from server %d. Owned by %d\n", id, hosts[c.getSocket()].id, locks[id]->owner);
      }
   } else if (op == ops::release) { // send release from self to remote host!
      //printf("%d packet Release\n", ownId);
      if (!lockExists(id)) {
         printf("Release for lock that doesn't exist: %d\n", id);
         c.send(Packet().writeInt(ops::failure).writeInt(id));
         return;
      }
      m.lock_shared();
      if (c.getSocket() == toSelf.getSocket() && locks[id]->owner != -1) {
         //printf("%d sent release to remote host %d\n", ownId, locks[id]->owner);
		 /**if (!hostExists(locks[id]->owner)) {
			printf("host does not exist for id: %d\n",id);
			return;
		 }**/
         hosts[idToSocket[locks[id]->owner]].c.send(p);
         if (lockData) {
            lockData->sendLockData(id, hosts[idToSocket[locks[id]->owner]].c);
         }
         m.unlock_shared();
      } else if (locks[id]->owner == -1) {
         if (lockData && !lockData->recvLockData(id, c)) {
            printf("Failed receiving lock data on release\n");
            m.unlock_shared();
         }
         if (locks[id]->waitList.size() > 0) {
            //printf("%d pulling off waitlist\n", ownId);
            int notify = -2;
            while (locks[id]->waitList.size() > 0 && !hostExists(notify)) {
               notify = locks[id]->waitList.front();
               locks[id]->waitList.pop();
            }
            m.unlock_shared();
            if (!hostExists(notify)) {
               printf("no hosts available on wait list\n");
               return;
            }
            Connection n = hosts[notify].c;
            n.send(p.reset().writeInt(ops::success).writeInt(id));
            if (lockData)
               lockData->sendLockData(id, n);
            //printf("1pulled %d off wait list\n", notify);
         } else {
            //printf("%d unlocking, no waitlist\n", ownId);
            locks[id]->m.unlock();
            m.unlock_shared();
         }
      } else {
         printf("Got release for lock %d from server %d. owned by %d\n", id, hosts[c.getSocket()].id, locks[id]->owner);
         m.unlock_shared();
      }
      //printf("released %d\n", id);
   } else if (op == ops::success) {
      //printf("%d forwarding success to self\n", ownId);
      if (lockData && !lockData->recvLockData(id, c)) {
         printf("Failed receiving lock data on release\n");
         m.unlock_shared();
         toSelf.send(Packet().writeInt(ops::failure).writeInt(id));
         return;
      }
      localWaitingOn = -1;
      toSelf.send(p);
   } else if (op == ops::available) {
      //printf("%d packet Available\n", ownId);
      m.lock_shared();
      //printf("asd\n");
      if (locks[id]->waitList.size() > 0 && locks[id]->m.try_lock()) {
         //printf("in here\n");
         int notify = -2;
         while (locks[id]->waitList.size() > 0 && !hostExists(notify)) {
            notify = locks[id]->waitList.front();
            locks[id]->waitList.pop();
         }
         m.unlock_shared();
         if (!hostExists(notify)) {
            printf("no hosts available on wait list\n");
            return;
         }
         Connection n = hosts[notify].c;
         n.send(p.reset().writeInt(ops::success).writeInt(id));
         if (lockData) {
            lockData->sendLockData(id, n);
         }
         //printf("2pulled %d off wait list\n", notify);
      } else 
         m.unlock_shared();
   } else if (op == ops::newHost) {
      //printf("%d Got new host packet.\n", ownId);
      Packet p2;
      if (!c.recv(p2, id)) { // id is the size of the remaining payload
         printf("Failed to recv all of newHost packet\n");
         return;
      }
      string host;
      int port;
      p2.readStdStr(host).readInt(port).readInt(id); // now id is the remote id
      //printf("host: %s : %d - %d\n", host.c_str(), port, id);
      Connection newHost(host.c_str(), port);
      if (newHost) {
         hosts[newHost.getSocket()] = Host(newHost, id);
         idToSocket[id] = newHost.getSocket();
         ss.add(newHost.getSocket());
         newHost.send(p2.reset().writeInt(ops::connect).writeInt(ownId));
         //printf("Host added.\n");
      } else {
         printf("Couldn't connect\n");
      }
   } else if (op == ops::connect) {
      hosts[c.getSocket()].id = id;
      idToSocket[id] = c.getSocket();
      ss.add(c.getSocket());
      printf("%d Lockmanager %d now identified\n", ownId, id);
   } else if (op == ops::failure) {
      printf("got a failure for id %d\n", id);
      localWaitingOn = -1;
      toSelf.send(p);
   } else if (op == ops::shutdown) {
      for (map<int, Host>::iterator itr = hosts.begin(); itr != hosts.end(); ++itr)
         itr->second.c.close();
      exitNow = true;
   } else {
      printf("Packet that was not identified!\n");
   }
   //printf("left\n");
}

bool LMData::lockExists(int id)
{
   //m.lock_shared();
   map<int, Lock*>::iterator itr = locks.find(id);
   bool ret = itr != locks.end();
   //m.unlock_shared();
   return ret;
}

bool LMData::hostExists(int socket)
{
   //m.lock_shared();
   map<int, Host>::iterator itr = hosts.find(socket);
   bool ret = itr != hosts.end() && itr->second.id != -2;
   //m.unlock_shared();
   return ret;
}

void LockManager::addLocalLock(int id)
{
   data->m.lock();
   data->locks[id] = new Lock(-1); // -1 means that the owner is the local machine
   data->m.unlock();
}

void LockManager::addRemoteLock(int id, int host)
{
   data->m.lock();
   data->locks[id] = new Lock(host);
   data->m.unlock();
}

bool LockManager::acquire(int id)
{
   //printf("%dc in acquire\n", data->ownId);
   data->m.lock_shared();

   if (!data->lockExists(id))
      return false;

   Lock *l = data->locks[id];

   if (l->owner == -1) {
      // Local machine is owner, simply get mutex;
      //printf("m: locking\n");
      l->m.lock();
      //printf("m: got it\n");
      data->m.unlock_shared();
   } else {
      // Remote machine is owner. Send request to thread
      data->m.unlock_shared();
      Packet p;
      data->self.send(p.writeInt(ops::acquire).writeInt(id));
      //printf("%dc waiting on success\n", data->ownId);
      if (!data->self.recv(p, 8)) {
         printf("recv failed waiting on lock success for %d\n", id);
         return false;
      }
      //printf("%dc got it!\n", data->ownId);
      int i, sid;
      p.readInt(i).readInt(sid);
      if (i == ops::failure) {
         return false;
      } if (i != ops::success) {
         printf("Not a success packet on acquire\n");
         return false;
      } else if (sid != id) {
         printf("Not the id requested. Asked for %d, got %d\n", id, sid);
         return false;
      }
   }
   return true;
}

void LockManager::release(int id)
{
   data->m.lock_shared();
   
   if (!data->lockExists(id))
      return;

   Lock *l = data->locks[id];
   if (l->owner == -1) {
      l->m.unlock();
      if (l->waitList.size() > 0) {
         //printf("sending available\n");
         data->self.send(Packet().writeInt(ops::available).writeInt(id));
      }
      data->m.unlock_shared();
   } else {
      data->m.unlock_shared();
      Packet p;
      data->self.send(p.writeInt(ops::release).writeInt(id));
   }
}

void LockManager::deleteLock(int id)
{
   data->m.lock();
   delete data->locks[id];
   data->locks.erase(data->locks.find(id));
   data->m.unlock();
}

void LockManager::addHost(const char *hostname, int port, int id)
{
   Packet p;
   // 0 is so that it takes the same space a
   data->self.send(p.writeInt(ops::newHost).writeInt(strlen(hostname)+9).writeCStr(hostname).writeInt(port).writeInt(id));
}

void LockManager::shutDown()
{
   data->self.send(Packet().writeInt(ops::shutdown).writeInt(0));
   data->t.join();
}