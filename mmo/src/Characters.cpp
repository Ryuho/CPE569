#include "Characters.h"
#include "World.h"
#include "Constants.h"
#include <cstdio>

using namespace constants;
using namespace std;
using namespace geom;

int stopAfterTicks = 100;

////////////////// PLAYER //////////////////
////////////////////////////////////////////

Player::Player(int id, vec2 pos, vec2 dir, int hp)
   : id(id), pos(pos), dir(dir), moving(false), alive(true), hp(hp), pvp(false)
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

void Player::update()
{
   if (moving && getTicks() - lastUpdate < predictTicks) {
      pos = pos + dir * getDt() * playerSpeed;
   } else {
      moving = false;
   }
}

////////////////// MISSILE //////////////////
/////////////////////////////////////////////

Missile::Missile(int id, int type, vec2 pos, vec2 dir)
   : id(id), type(type), pos(pos), alive(true)
{
   lastUpdate = getTicks();
   this->dir = dir;
   if (this->dir.length() > 0.0)
      this->dir = normalize(this->dir);
}

void Missile::update()
{
   pos = pos + dir * projectileSpeed * getDt();
}

void Missile::move(vec2 pos, vec2 dir)
{
   this->lastUpdate = getTicks();
   this->pos = pos;
   this->dir = dir;
}

////////////////// ITEM //////////////////
//////////////////////////////////////////

Item::Item(int id, int type, vec2 pos) 
   : id(id), type(type), pos(pos)
{
    lastUpdate = getTicks();
    alive = true;
    initGraphics();
}

void Item::update()
{

}

void Item::move(vec2 pos)
{
   this->lastUpdate = getTicks();
   this->pos = pos;
}

////////////////// NPC //////////////////
/////////////////////////////////////////

NPC::NPC(int id, int type, int hp, vec2 pos, vec2 dir, bool moving)
   : id(id), type(type), pos(pos), dir(dir), moving(moving), hp(hp)
{
   alive = true;
   initGraphics();
   lastUpdate = getTicks();
}

void NPC::update()
{
   if (moving && getTicks() - lastUpdate < predictTicks) {
      pos = pos + dir * getDt() * npcWalkSpeed;
   } else {
      moving = false;
   }
}

void NPC::move(vec2 pos, vec2 dir, bool moving)
{
   this->lastUpdate = getTicks();
   this->pos = pos;
   this->dir = dir;
   if(!this->moving && moving) {
      anim->animStart = getTicks();
   }
   this->moving = moving;
}

////////////////// OBJECTHOLDER //////////////////
//////////////////////////////////////////////////

void ObjectHolder::addPlayer(Player p)
{
   if (!checkObject(p.id, ObjectType::Player)) {
      idToIndex[p.id] = IdType(players.size(), ObjectType::Player);
      players.push_back(p);
   }
}

void ObjectHolder::addMissile(Missile m)
{
   if (!checkObject(m.id, ObjectType::Missile)) {
      idToIndex[m.id] = IdType(missiles.size(), ObjectType::Missile);   
      missiles.push_back(m);
   }
}

void ObjectHolder::addItem(Item i)
{
   if (!checkObject(i.id, ObjectType::Item)) {
      idToIndex[i.id] = IdType(items.size(), ObjectType::Item);
      items.push_back(i);
   }
}

void ObjectHolder::addNPC(NPC n)
{
   if (!checkObject(n.id, ObjectType::NPC)) {
      idToIndex[n.id] = IdType(npcs.size(), ObjectType::NPC);
      npcs.push_back(n);
   }
}


Player *ObjectHolder::getPlayer(int id)
{
   if (!checkObject(id, ObjectType::Player)) {
      printf("Attempting to access Player that doesn't exist!\n");
      return 0;
   }
   return &players[idToIndex[id].index];
}

Missile *ObjectHolder::getMissile(int id)
{
   if (!checkObject(id, ObjectType::Missile)) {
      printf("Attempting to access Missile that doesn't exist!\n");
      return 0;
   }
   return &missiles[idToIndex[id].index];
}

Item *ObjectHolder::getItem(int id)
{
   if (!checkObject(id, ObjectType::Item)) {
      printf("Attempting to access Item that doesn't exist!\n");
      return 0;
   }
   return &items[idToIndex[id].index];
}

NPC *ObjectHolder::getNPC(int id)
{
   if (!checkObject(id, ObjectType::NPC)) {
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

   if (idToIndex[id].type == ObjectType::Player)
      removeTempl(idToIndex, players, id);

   else if (idToIndex[id].type == ObjectType::Missile)
      removeTempl(idToIndex, missiles, id);

   else if (idToIndex[id].type == ObjectType::Item)
      removeTempl(idToIndex, items, id);

   else if (idToIndex[id].type == ObjectType::NPC)
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

void ObjectHolder::updateAll()
{
   updateTempl(players, *this);
   updateTempl(missiles, *this);
   updateTempl(items, *this);
   updateTempl(npcs, *this);
}

void ObjectHolder::drawAll()
{
   int ticks = getTicks();
   for (unsigned i = 0; i < items.size(); i++)
      items[i].draw();

   for (unsigned i = 0; i < missiles.size(); i++)
      missiles[i].draw();
   
   for (unsigned i = 0; i < npcs.size(); i++) {
      if(ticks - npcs[i].lastUpdate < noDrawTicks)
         npcs[i].draw();
   }

   for (unsigned i = 0; i < players.size(); i++) {
      if(ticks - players[i].lastUpdate < noDrawTicks)
         players[i].draw();
   }

   for (unsigned i = 0; i < border.size(); i++)
      border[i].draw();
}

