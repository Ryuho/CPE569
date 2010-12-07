#include "BotWorld.h"
#include "BotCharacters.h"
#include "Constants.h"
#include <cstdio>

using namespace std;
using namespace geom;
using namespace constants;
using namespace botclient;

namespace botclient {

Player::Player(int id, vec2 pos, vec2 dir, int hp)
   : PlayerBase(id, pos), dir(dir), hp(hp), moving(false), alive(true)
{
   lastUpdate = getTicks();
}

void Player::move(vec2 pos, vec2 dir, bool moving)
{
   this->pos = pos;
   this->dir = dir;
   this->moving = moving;
}

void Player::update()
{
   lastUpdate = getTicks();
}

Missile::Missile(int id, int type, vec2 pos, vec2 dir)
   : MissileBase(id, type, pos), alive(true)
{
   this->dir = dir;
   if (this->dir.length() > 0.0f)
      this->dir = normalize(this->dir);
}

void Missile::update()
{
   pos = pos + dir * projectileSpeed * getDt();
}

Item::Item(int id, int type, vec2 pos) 
   : ItemBase(id, type, pos), alive(true)
{
   lastUpdate = getTicks();
}

void Item::update()
{
   lastUpdate = getTicks();
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

NPC::NPC(int id, int type, int hp, vec2 pos, vec2 dir, bool moving)
   : NPCBase(id, type, pos), hp(hp), dir(dir), moving(moving), alive(true)
{
   lastUpdate = getTicks();
}

void NPC::update()
{
   lastUpdate = getTicks();
}

////////////////////////////////////
/////////// ObjectHolder ///////////
////////////////////////////////////

bool ObjectHolder::addPlayer(Player *obj)
{
   return add(static_cast<PlayerBase *>(obj));
}

bool ObjectHolder::addMissile(Missile *obj)
{
   return add(static_cast<MissileBase *>(obj));
}

bool ObjectHolder::addItem(Item *obj)
{
   return add(static_cast<ItemBase *>(obj));
}

bool ObjectHolder::addNPC(NPC *obj)
{
   return add(static_cast<NPCBase *>(obj));
}

Player *ObjectHolder::getPlayer(int id)
{
   return static_cast<Player *>(rm.getObject(id));
}

Missile *ObjectHolder::getMissile(int id)
{
   return static_cast<Missile *>(rm.getObject(id));
}

Item *ObjectHolder::getItem(int id)
{
   return static_cast<Item *>(rm.getObject(id));
}

NPC *ObjectHolder::getNPC(int id)
{
   return static_cast<NPC *>(rm.getObject(id));
}

void ObjectHolder::updateAll()
{
   for(unsigned i = 0; i < playerCount(); i++) {
      Player &obj = *static_cast<Player *>(get(ObjectType::Player, i));
      obj.update();
   }
   for(unsigned i = 0; i < npcCount(); i++) {
      NPC &obj = *static_cast<NPC *>(get(ObjectType::NPC, i));
      obj.update();
   }
   for(unsigned i = 0; i < itemCount(); i++) {
      Item &obj = *static_cast<Item *>(get(ObjectType::Item, i));
      obj.update();
   }
   for(unsigned i = 0; i < missileCount(); i++) {
      Missile &obj = *static_cast<Missile *>(get(ObjectType::Missile, i));
      obj.update();
   }
}

}