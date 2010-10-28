#include "Characters.h"
#include "GameUtil.h"
#include "Constants.h"

using namespace game;
using namespace constants;
using namespace std;

   int stopAfterTicks = 100;

void Player::setPos(mat::vec2 pos)
{
   if (!alive) {
      alive = true;
      dir = vec2(0.0, 1.0);
   }

   moving = false;
   this->pos = pos;
}

void Player::moveTo(mat::vec2 pos)
{
   if (!alive)
      setPos(pos);

   if (!moving) {
      animStart = getTicks();
   }

   vec2 newDir = to(this->pos, pos);
   if (newDir.length() > 0.0)
      dir = normalize(newDir);

   this->pos = pos;
   moving = true;
   lastUpdate = getTicks();
}

void Player::update()
{
   if (getTicks() - lastUpdate < playerPredictTicks) {
      pos = pos + dir * getDt() * playerSpeed;
   } else {
      moving = false;
   }
}

void Missile::init(mat::vec2 pos, mat::vec2 dir, Missile::Type type)
{
   start = pos;
   if (dir.length() > 0.0)
      this->dir = normalize(dir);
   else
      this->dir = vec2(0.0, 1.0);
   this->type = type;

   this->pos = start + this->dir * 75.0;

   alive = true;
}

void Missile::update()
{
   if (alive) {
      pos = pos + dir * projectileSpeed * getDt();

      if (dist(start, pos) > maxProjectileDist)
         alive = false;
   }
}

void Item::init(vec2 pos, Type type)
{
    this->pos = pos;
    this->type = type;
    alive = true;

    initGraphics();
}

void Item::update()
{
   if (alive) {
      //if within 40 pixels make not alive
      if (dist(getPlayer().pos, pos) < 40)
         alive = false;
   }
}

void NPC::init(vec2 pos, Type type)
{
   this->type = type;
   this->pos = pos;
   moving = false;
   alive = true;
   dir = vec2(0, 0);

   initGraphics();
}

void NPC::update()
{
   if(alive) {
      vec2 dist = to(pos, getPlayer().pos);
      bool ismoving = dist.length() > 40;
      if(!moving && ismoving)
         resetAnimation();
      moving = ismoving;
      dir = normalize(dist);
      if(moving)
         this->pos = dir*npcSpeed*getDt() + pos;
   }
}


void ObjectHolder::addPlayer(Player p)
{
   if (!checkObject(p.id, IdType::Player)) {
      players.push_back(p);
      idToIndex[p.id] = IdType(players.size()-1, IdType::Player);
   }
}

void ObjectHolder::addMissile(Missile m)
{
   //if (!checkObject(m.id, IdType::Missile)) { // This should be put back in once checkObject is fixed
      missiles.push_back(m);
      idToIndex[m.id] = IdType(missiles.size()-1, IdType::Missile);
   //}
}

void ObjectHolder::addItem(Item i)
{   
   if (!checkObject(i.id, IdType::Item)) {
      items.push_back(i);
      idToIndex[i.id] = IdType(items.size()-1, IdType::Item);
   }
}

void ObjectHolder::addNPC(NPC n)
{
   if (!checkObject(n.id, IdType::NPC)) {
      npcs.push_back(n);
      idToIndex[n.id] = IdType(npcs.size()-1, IdType::NPC);
   }
}


Player &ObjectHolder::getPlayer(int id)
{
   if (!checkObject(id, IdType::Player))
      printf("Attempting to access Player that doesn't exist!\n");
   return players[idToIndex[id].index];
}

Missile &ObjectHolder::getMissile(int id)
{
   if (!checkObject(id, IdType::Missile))
      printf("Attempting to access Missile that doesn't exist!\n");
   return missiles[idToIndex[id].index];
}

Item &ObjectHolder::getItem(int id)
{
   if (!checkObject(id, IdType::Item))
      printf("Attempting to access Item that doesn't exist!\n");
   return items[idToIndex[id].index];
}

NPC &ObjectHolder::getNPC(int id)
{
   if (!checkObject(id, IdType::NPC))
      printf("Attempting to access NPC that doesn't exist!\n");
   return npcs[idToIndex[id].index];
}

// Checks to see if the object exists already
bool ObjectHolder::checkObject(int id, int type)
{
   map<int,IdType>::iterator itr = idToIndex.find(id);
   if (itr == idToIndex.end()) {
      // Normally the behavior here should be an error,
      // but for the moment we should just add a new object of that type.
      // Later on lets make object creation explicit
      const char *typeStr = 
         type == IdType::Player  ? "Player" :
         type == IdType::Missile ? "Missile" :
         type == IdType::Item    ? "Item" :
         type == IdType::NPC     ? "NPC" :
                                   "Unknown type";
      printf("Object with id %d not found in objects, creating an uninitalized %s\n", id, typeStr);
      if (type == IdType::Player) {
         idToIndex[id] = IdType(players.size(), IdType::Player);
         players.push_back(Player(id));
      } else if (type == IdType::Missile) {
         idToIndex[id] = IdType(missiles.size(), IdType::Missile);
         missiles.push_back(Missile(id));
      } else if (type == IdType::Item) {
         idToIndex[id] = IdType(items.size(), IdType::Item);
         items.push_back(Item(id));
      } else if (type == IdType::NPC) {
         idToIndex[id] = IdType(npcs.size(), IdType::NPC);
         npcs.push_back(NPC(id));
      }

      return true;
      //return false;
   } else if (itr->second.type != type) {
      printf("Object with id %id already exists with different type! %d %d\n", type, itr->second.type);
      return false;
   }
   return true;
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

void ObjectHolder::updateAll()
{
   updateTempl(players, *this);
   updateTempl(missiles, *this);
   updateTempl(items, *this);
   updateTempl(npcs, *this);
}

void ObjectHolder::drawAll()
{
   for (unsigned i = 0; i < players.size(); i++)
      players[i].draw();

   for (unsigned i = 0; i < missiles.size(); i++)
      missiles[i].draw();
   
   for (unsigned i = 0; i < items.size(); i++)
      items[i].draw();

   for (unsigned i = 0; i < npcs.size(); i++)
      npcs[i].draw();
}










