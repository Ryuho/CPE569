#include "BotWorld.h"
#include "BotCharacters.h"
#include "Constants.h"
#include <cstdio>

using namespace std;
using namespace geom;
using namespace constants;
using namespace botclient;

namespace botclient {

////////////////// PLAYER //////////////////
////////////////////////////////////////////

Player::Player(int id, vec2 pos, vec2 dir, int hp)
   : PlayerBase(id, pos), dir(dir), moving(false), hp(hp),
   rupees(0), exp(0), pvp(false)
{
   lastUpdate = getTicks();
}

void Player::move(vec2 pos, vec2 dir, bool moving)
{
   lastUpdate = getTicks();
   this->pos = pos;
   this->dir = dir;
   this->moving = moving;
}

void Player::update()
{
   if (moving && getTicks() - lastUpdate < predictTicks) {
      pos = pos + dir * getDt() * playerSpeed;
   } 
   else {
      moving = false;
   }
}

////////////////// MISSILE //////////////////
/////////////////////////////////////////////

Missile::Missile(int id, int type, vec2 pos, vec2 dir)
   : MissileBase(id, type, pos)
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
   : ItemBase(id, type, pos), alive(true)
{
   lastUpdate = getTicks();
}

void Item::update()
{

}

bool Item::isCollectable() const
{
   switch (type) {
      case ItemType::GreenRupee:
      case ItemType::RedRupee:
      case ItemType::BlueRupee:
      case ItemType::Heart:
         return true;
      case ItemType::Explosion:
      case ItemType::Stump:
         break;
      default:
         printf("Error Item::isCollectable - Unknown item type %d\n", type);
   }
   return false;
}

void Item::move(vec2 pos)
{
   this->lastUpdate = getTicks();
   this->pos = pos;
}

////////////////// NPC //////////////////
/////////////////////////////////////////

NPC::NPC(int id, int type, int hp, vec2 pos, vec2 dir, bool moving)
   : NPCBase(id, type, pos), dir(dir), moving(moving), hp(hp)
{
   lastUpdate = getTicks();
}

void NPC::update()
{
   if (moving && getTicks() - lastUpdate < predictTicks) {
      pos = pos + dir * getDt() * npcWalkSpeed;
   } 
   else {
      moving = false;
   }
}

void NPC::move(vec2 pos, vec2 dir, bool moving)
{
   this->lastUpdate = getTicks();
   this->pos = pos;
   this->dir = dir;
   this->moving = moving;
}

////////////////// OBJECTHOLDER //////////////////
//////////////////////////////////////////////////

bool ObjectHolder::add(Player *obj)
{
   return ObjectManager::add(static_cast<PlayerBase *>(obj));
}

bool ObjectHolder::add(Missile *obj)
{
   return ObjectManager::add(static_cast<MissileBase *>(obj));
}

bool ObjectHolder::add(Item *obj)
{
   return ObjectManager::add(static_cast<ItemBase *>(obj));
}

bool ObjectHolder::add(NPC *obj)
{
   return ObjectManager::add(static_cast<NPCBase *>(obj));
}

Player *ObjectHolder::getPlayer(int id)
{
   return static_cast<Player *>(rm.get(id));
}

Missile *ObjectHolder::getMissile(int id)
{
   return static_cast<Missile *>(rm.get(id));
}

Item *ObjectHolder::getItem(int id)
{
   return static_cast<Item *>(rm.get(id));
}

NPC *ObjectHolder::getNPC(int id)
{
   return static_cast<NPC *>(rm.get(id));
}

Player *ObjectHolder::getPlayerByIndex(int index) const
{
   return static_cast<Player *>(get(ObjectType::Player, index));
}

NPC *ObjectHolder::getNPCByIndex(int index) const
{
   return static_cast<NPC *>(get(ObjectType::NPC, index));
}

Item *ObjectHolder::getItemByIndex(int index) const
{
   return static_cast<Item *>(get(ObjectType::Item, index));
}

Missile *ObjectHolder::getMissileByIndex(int index) const
{
   return static_cast<Missile *>(get(ObjectType::Missile, index));
}

void ObjectHolder::updateAll()
{
   for(unsigned i = 0; i < playerCount(); i++) {
      Player &obj = *getPlayerByIndex(i);
      obj.update();
   }
   for(unsigned i = 0; i < npcCount(); i++) {
      NPC &obj = *getNPCByIndex(i);
      obj.update();
   }
   for(unsigned i = 0; i < itemCount(); i++) {
      Item &obj = *getItemByIndex(i);
      obj.update();
   }
   for(unsigned i = 0; i < missileCount(); i++) {
      Missile &obj = *getMissileByIndex(i);
      obj.update();
   }
}

}