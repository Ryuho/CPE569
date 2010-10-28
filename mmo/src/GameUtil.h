#ifndef _GAME_UTIL_H_
#define _GAME_UTIL_H_

// These functions provide access to useful game related actions

namespace game {
   int getTicks();
   float getDt();
   Player &getPlayer(); // available only on client
};

#endif