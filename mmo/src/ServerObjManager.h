#ifndef _SERVER_OM_H_
#define _SERVER_OM_H_

#include "Constants.h"
#include "matrix.h"
#include "packet.h"
#include "Geometry.h"
#include "ServerData.h"
#include <vector>
#include <map>

namespace server {
   using namespace std;
   using namespace mat;
   using namespace constants;
   using namespace geom;

  struct Region {
     vector<Player *> players;
     vector<Missile *> missiles;
     vector<NPC *> npcs;
     vector<Item *> items;

     void add(Player *p);
     void add(NPC *p);
     void add(Missile *p);
     void add(Item *p);

     void remove(Player *p);
     void remove(NPC *n);
     void remove(Missile *m);
     void remove(Item *i);

     bool contains(Player *p);
     bool contains(NPC *p);
     bool contains(Missile *p);
     bool contains(Item *p);
  };

  struct ObjectManager {
     struct Index {
        Index() {}
        Index(int index, int type) : index(index), type(type) {}
        int index, type;
     };
     
     void init(float width, float height, float regionWidth);
     bool inBounds(vec2 pos) const;

     Player *getPlayer(int id);
     Missile *getMissile(int id);
     NPC *getNpc(int id);
     Item *getItem(int id);

     vector<Player *> collidingPlayers(Geometry g, const vec2 &center);
     vector<Missile *> collidingMissiles(Geometry g, const vec2 &center);
     vector<NPC *> collidingNPCs(Geometry g, const vec2 &center);
     vector<Item *> collidingItems(Geometry g, const vec2 &center);

     void add(Player *p);
     void add(Missile *m);
     void add(NPC *n);
     void add(Item *i);

     void move(Player *p, const vec2 &newPos);
     void move(Item *i, const vec2 &newPos);
     void move(Missile *m, const vec2 &newPos);
     void move(NPC *n, const vec2 &newPos);

     void getRegion(vec2 pos, int &x, int &y);
     void getRegions(vec2 pos, Geometry g, std::vector<Region *> &regs);
     Geometry getRegionGeom(int x, int y);

     void remove(int id);
     bool check(int id, int type);

     map<int, Index> idToIndex;
     vector<Player *> players;
     vector<Missile *> missiles;
     vector<NPC *> npcs;
     vector<Item *> items;
     
     vector<vector<Region> > regions; //starts botLeft and increases in <X,Y> 

     map<int,int> oldTypes;
     vec2 worldBotLeft;
     float regionSize;
  };
} // end server namespace

#endif
