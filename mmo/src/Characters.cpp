#include "Characters.h"
#include "World.h"
#include "Constants.h"
#include <cstdio>

using namespace constants;
using namespace std;
using namespace geom;

int stopAfterTicks = 100;

Player::Player(int id, vec2 pos, vec2 dir, int hp)
   : id(id), pos(pos), dir(dir), moving(false), alive(true), hp(hp)
{
   lastUpdate = getTicks();
}

void Player::move(vec2 pos, vec2 dir, bool moving)
{
   lastUpdate = getTicks();
   if (moving && !this->moving)
      animStart = getTicks();

   this->pos = pos;
   this->dir = dir;
   this->moving = moving;
}

void Player::stop()
{
   moving = false;
}

void Player::update()
{
   if (moving && getTicks() - lastUpdate < playerPredictTicks) {
      pos = pos + dir * getDt() * playerSpeed;
   } else {
      moving = false;
   }
}

Missile::Missile(int id, int type, vec2 pos, vec2 dir)
   : id(id), type(type), pos(pos)
{
   this->dir = dir;
   if (this->dir.length() > 0.0)
      this->dir = normalize(this->dir);
}

void Missile::update()
{
   pos = pos + dir * projectileSpeed * getDt();
}

Item::Item(int id, int type, vec2 pos) 
   : id(id), type(type), pos(pos)
{
    alive = true;
    initGraphics();
}

void Item::update()
{

}

NPC::NPC(int id, int type, vec2 pos, vec2 dir, bool moving)
   : id(id), type(type), pos(pos), dir(dir), moving(moving)
{
   alive = true;
   initGraphics();
}

void NPC::update()
{

}


void ObjectHolder::addPlayer(Player p)
{
   if (!checkObject(p.id, IdType::Player)) {
      idToIndex[p.id] = IdType(players.size(), IdType::Player);
      players.push_back(p);
   }
}

void ObjectHolder::addMissile(Missile m)
{
   if (!checkObject(m.id, IdType::Missile)) {
      idToIndex[m.id] = IdType(missiles.size(), IdType::Missile);   
      missiles.push_back(m);
   }
}

void ObjectHolder::addItem(Item i)
{
   if (!checkObject(i.id, IdType::Item)) {
      idToIndex[i.id] = IdType(items.size(), IdType::Item);
      items.push_back(i);
   }
}

void ObjectHolder::addNPC(NPC n)
{
   if (!checkObject(n.id, IdType::NPC)) {
      idToIndex[n.id] = IdType(npcs.size(), IdType::NPC);
      npcs.push_back(n);
   }
}


Player *ObjectHolder::getPlayer(int id)
{
   if (!checkObject(id, IdType::Player)) {
      printf("Attempting to access Player that doesn't exist!\n");
      return 0;
   }
   return &players[idToIndex[id].index];
}

Missile *ObjectHolder::getMissile(int id)
{
   if (!checkObject(id, IdType::Missile)) {
      printf("Attempting to access Missile that doesn't exist!\n");
      return 0;
   }
   return &missiles[idToIndex[id].index];
}

Item *ObjectHolder::getItem(int id)
{
   if (!checkObject(id, IdType::Item)) {
      printf("Attempting to access Item that doesn't exist!\n");
      return 0;
   }
   return &items[idToIndex[id].index];
}

NPC *ObjectHolder::getNPC(int id)
{
   if (!checkObject(id, IdType::NPC)) {
      printf("Attempting to access NPC that doesn't exist!\n");
      return 0;
   }
   return &npcs[idToIndex[id].index];
}

// Checks to see if the object exists already
bool ObjectHolder::checkObject(int id, int type)
{
   map<int,IdType>::iterator itr = idToIndex.find(id);
   return itr != idToIndex.end() && itr->second.type == type;
   //   printf("Object with id %id already exists with different type! existing=%d new=%d\n", id, type, itr->second.type);
}

template<typename T>
void removeTempl(map<int, ObjectHolder::IdType> &idToIndex, vector<T> &objs, int id)
{
   int i = idToIndex[id].index;

   idToIndex.erase(id);
   if (objs.size() > 1) {
      objs[i] = objs.back();
      idToIndex[objs[i].id].index = i;
   }
   objs.pop_back();
}

void ObjectHolder::removeObject(int id)
{
   int i = idToIndex[id].index;

   if (idToIndex[id].type == IdType::Player)
      removeTempl(idToIndex, players, id);

   else if (idToIndex[id].type == IdType::Missile)
      removeTempl(idToIndex, missiles, id);

   else if (idToIndex[id].type == IdType::Item)
      removeTempl(idToIndex, items, id);

   else if (idToIndex[id].type == IdType::NPC)
      removeTempl(idToIndex, npcs, id);
}

// Didn't make this a member function because ugly
// and nobody else needs to use it.
template<typename T>
void updateTempl(vector<T> &objs, ObjectHolder &o)
{
   for (unsigned i = 0; i < objs.size(); i++) {
      objs[i].update();
      if (!objs[i].alive) {
         o.removeObject(objs[i].id);
         i--;
      }
   }
}

//Server update code for NPCs
//passed in a list of close players, hostile players, or whatever
//if empty, then do not move unless its in the AI
void NPC::updateServer(std::vector<vec2> dests)
{
   //find closest player
   if(!alive) {
      printf("Cannot update NPC %d: not alive\n", id);
      return;
   }

   /*
   //AI part, will eventually be changed with plugabble AI
   vec2 dest(pos);
   float length = 2000.0; //would use MAX_FLOAT but I'm too lazy to include
   if(alive) {
      for(unsigned i = 0; i < dests.size(); i++) {
         float d = dist(pos, dests[i]);
         if(d < length && d > 500.0) {
            length = d;
            dest = dests[i];
            dir = mat::to(pos, dests[i]).normalize();
         }
      }
      moving = length > 0;
      if(moving)
         pos = dir*npcSpeed*getDt() + pos;
   }
   */


   //AI part, will eventually be changed with plugabble AI
   if(alive) {
      if(dir.length() < 0.5 || dir.length() > 2) {
         dir = vec2((float)(rand()%500), (float)(rand()%500)).normalize();
      }
      float d = mat::dist(pos, vec2(0,0));
      if(d > 800.0 || d < -800.0) {
         pos = vec2(0,0);
         if(dests.size() != 0) {
            vec2 dest(mat::to(pos, dests[rand() % dests.size()]));
            if(dest.length() > 0.1 || dest.length() < -0.1)
               dir = dest.normalize();
            else
               dir = vec2((float)(rand()%500), (float)(rand()%500)).normalize();
         }
      }
      pos = dir*npcSpeed*getDt() + pos;
   }
}

void ObjectHolder::updateAll()
{
   updateTempl(players, *this);
   updateTempl(missiles, *this);
   updateTempl(items, *this);
   updateTempl(npcs, *this);
}

void ObjectHolder::drawAll()
{
   for (unsigned i = 0; i < items.size(); i++)
      items[i].draw();

   for (unsigned i = 0; i < missiles.size(); i++)
      missiles[i].draw();
   
   for (unsigned i = 0; i < npcs.size(); i++)
      npcs[i].draw();

   for (unsigned i = 0; i < players.size(); i++)
      players[i].draw();
}

