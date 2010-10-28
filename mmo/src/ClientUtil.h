#ifndef _CLIENT_UTIL_H_
#define _CLIENT_UTIL_H_

// These functions provide access to the current
// client state. Their implementations are in World.cpp

namespace client {
   int getTicks();
   float getDt();
   Player &getPlayer();
};

#endif