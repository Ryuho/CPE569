#include "ServerData.h"
#include "GameServer.h"

namespace server {
using namespace mat;
using namespace constants;


// Player

Player::Player(int id, vec2 pos, vec2 dir)
   : id(id), pos(pos), dir(dir), moving(false)
{
   
}


void Player::move(vec2 pos, vec2 dir, bool moving)
{
   this->pos = pos;
   this->dir = dir;
   this->moving = moving;
}


// Missile

Missile::Missile(int id, mat::vec2 pos, mat::vec2 dir, int type)
   : id(id), pos(pos), dir(dir), type(type)
{
   spawnTime = getTicks();
}


void Missile::update()
{
   pos = pos + dir * projectileSpeed * getDt();
}


// Object Manager

void ObjectManager::addPlayer(Player p)
{
   idToIndex[p.id] = Index(players.size(), ObjectType::Player);
   players.push_back(new Player(p));
}

void ObjectManager::addMissile(Missile m)
{
   idToIndex[m.id] = Index(missiles.size(), ObjectType::Missile);
   missiles.push_back(new Missile(m));
}

void ObjectManager::addNPC(NPC n)
{
   idToIndex[n.id] = Index(npcs.size(), ObjectType::NPC);
   npcs.push_back(new NPC(n));
}

void ObjectManager::addItem(Item i)
{
   idToIndex[i.id] = Index(items.size(), ObjectType::Item);
   items.push_back(new Item(i));
}

Player *ObjectManager::getPlayer(int id)
{
   return players[idToIndex[id].index];
}

Missile *ObjectManager::getMissile(int id)
{
   return missiles[idToIndex[id].index];
}

NPC *ObjectManager::getNpc(int id)
{
   return npcs[idToIndex[id].index];
}

Item *ObjectManager::getItem(int id)
{
   return items[idToIndex[id].index];
}

template<typename T>
void removeTempl(map<int, ObjectManager::Index> &idToIndex, vector<T*> &objs, int id)
{
   int i = idToIndex[id].index;

   delete objs[i];
   idToIndex.erase(id);
   if (objs.size() > 1) {
      objs[i] = objs.back();
      idToIndex[objs[i]->id].index = i;
   }
   objs.pop_back();
}

void ObjectManager::remove(int id)
{   int i = idToIndex[id].index;

   if (idToIndex[id].type == ObjectType::Player)
      removeTempl(idToIndex, players, id);

   else if (idToIndex[id].type == ObjectType::Missile)
      removeTempl(idToIndex, missiles, id);

   else if (idToIndex[id].type == ObjectType::NPC)
      removeTempl(idToIndex, npcs, id);

   else if (idToIndex[id].type == ObjectType::Item)
      removeTempl(idToIndex, items, id);
}


bool ObjectManager::check(int id, int type)
{
   map<int, Index>::iterator itr = idToIndex.find(id);

   return itr != idToIndex.end() && itr->second.type == type;
}


} // end server namespace