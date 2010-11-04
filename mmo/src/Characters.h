#ifndef _CHARACTERS_H_
#define _CHARACTERS_H_

#include <vector>
#include <map>
#include "matrix.h"
#include "Geometry.h"

using mat::vec2;
struct Animation;

void initCharacterResources();

struct Player {
   Player(int id) : id(id), alive(false) {}
   Player(int id, vec2 pos, vec2 dir);
   void move(vec2 pos, vec2 dir, bool moving);
   void stop();

   void update();
   void draw();
   geom::Circle bounds();

   vec2 dir, pos;
   float radius;
   bool moving, alive;
   int animStart, lastUpdate;
   int id;
};

struct Missile {
   enum Type { Arrow };
   Missile(int id) : id(id), alive(false) {}
   void init(vec2 pos, vec2 dir, int type);

   void update();
   void draw();

   int type;
   vec2 pos, dir, start;
   bool alive;
   int id;
};

struct Item {
   enum Type { GreenRupee, RedRupee, BlueRupee, Explosion,  MaxItem };
   Item(int id) : id(id), alive(false), anim(0), type(MaxItem) {}
   void init(vec2 pos, Type type);

   void initGraphics();
   void update();
   void draw();
   
   int type;
   vec2 pos;
   bool alive;
   Animation *anim;
   int id;
};

struct NPC {
   enum Type { Thief, Princess, Fairy, Skeleton, Cyclops, 
      Bat, Bird, Squirrel, Chicken, Vulture, Bush, Cactus, 
      BigFairy, Wizard, Ganon, Goblin, MaxNPC };
   NPC(int id) : id(id), alive(false) {}
   void init(vec2 pos, Type type);

   void initGraphics();
   void resetAnimation();
   void updateServer(std::vector<mat::vec2> dests);
   void update();
   void draw();
   geom::Circle bounds();
   
   int type;
   vec2 pos, dir;
   bool alive, moving;
   Animation *anim;
   int id;
};

struct ObjectHolder {
   struct IdType {
      enum { Player, Missile, Item, NPC };
      IdType() {}
      IdType(int index, int type) : index(index), type(type) {}
      int index, type;
   };

   std::vector<Player> players;
   std::vector<Missile> missiles;
   std::vector<Item> items;
   std::vector<NPC> npcs;
   std::map<int, IdType> idToIndex;

   void addPlayer(Player p);
   void addMissile(Missile m);
   void addItem(Item i);
   void addNPC(NPC n);

   Player *getPlayer(int id);
   Missile *getMissile(int id);
   Item *getItem(int id);
   NPC *getNPC(int id);

   bool checkObject(int id, int type);
   void removeObject(int id);

   void updateAll();
   void drawAll();
};

namespace game {
   int getTicks();
   float getDt();
   Player &getPlayer(); // available only on client
};

#endif
