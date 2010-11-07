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

struct LMData {
   LMData(Server serv) : serv(serv) {}
   void operator()();
   void handlePacket(Connection c);

   thread t;
   shared_mutex m; // mutex for modifying data structures
   map<int, Lock*> locks;
   Connection self;

private: // Thread private storage
   map<int, Connection> hosts;
   Server serv;
   Connection toSelf;
};

void LockManager::startThread(int port)
{
   setupSockets();
   Server serv(port);
   serv.listen(5);
   data = new LMData(serv);


   data->m.lock();
   data->self = Connection("localhost", serv.port());
   data->t = thread(ref(*data));
   data->m.unlock();
}

void LMData::operator()()
{
   SelectSet ss;
   serv.select(-1);
   
   toSelf = serv.accept(); // assuming first connection is local machine

   ss.add(serv.getSocket());
   ss.add(toSelf.getSocket());
   
   printf("t- ready\n");

   while (1) {
      vector<int> readySds = ss.select(-1);
      
      printf("t- selected %d\n", readySds.size());
      
      for (unsigned i = 0; i < readySds.size(); i++) {
         if (readySds[i] == serv.getSocket()) {
            Connection c = serv.accept();
            ss.add(c.getSocket());
            hosts[c.getSocket()] = c;
            printf("Got a new lock connection %d\n", c.getSocket());
         } else {
            Connection c = hosts[readySds[i]];
            if (c)
               handlePacket(hosts[readySds[i]]);
            else {
               ss.remove(c.getSocket());
               hosts.erase(hosts.find(c.getSocket()));
               printf("remote host %d disconnected\n", c.getSocket());
            }
         }
      }
   }

}

void LMData::handlePacket(Connection c)
{
   int op, id;
   Packet p;

   c.recv(p, 8);
   p.readInt(op).readInt(id);

   if (op == ops::acquire) {
      m.lock_upgrade();
      if (locks[id]->m.try_lock()) {
         m.unlock_upgrade();
         c.send(p.reset().writeInt(ops::success));
         printf("acquired %d\n", id);
      } else {
         m.unlock_upgrade_and_lock();
         locks[id]->waitList.push(c.getSocket());
         m.unlock();
         printf("put %d on wait list\n", c.getSocket());
      }
   } else if (op == ops::release) {
      m.lock_upgrade();
      locks[id]->m.unlock();
      if (locks[id]->waitList.size() > 0) {
         int notify = locks[id]->waitList.front();
         m.unlock_upgrade_and_lock();
         locks[id]->waitList.pop();
         m.unlock_and_lock_upgrade();
         Connection n = hosts[notify];
         n.send(p.reset().writeInt(ops::success));
         printf("pulled %d off wait list\n", notify);
      }
      m.unlock_upgrade();
      printf("released %d\n", id);
   }
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

void LockManager::acquire(int id)
{
   data->m.lock_shared();
   Lock *l = data->locks[id];

   if (l->owner == -1) {
      // Local machine is owner, simply get mutex;
      l->m.lock();
      data->m.unlock_shared();
   } else {
      // Remote machine is owner. Send request to thread
      data->m.unlock_shared();
      Packet p;
      data->self.send(p.writeInt(ops::acquire).writeInt(id));
      data->self.recv(p, 4);
      /*int i;
      p.readInt(i);
      if (i == ops::success)
         printf("got %d\n", id);
      else
         printf("couldn't get it\n");*/
   }
}

void LockManager::release(int id)
{
   data->m.lock_shared();
   Lock *l = data->locks[id];
   if (l->owner == -1) {
      l->m.unlock();
      if (l->waitList.size() > 0) {
         data->self.send(Packet().writeInt(ops::release).writeInt(id));
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
