#include "GameServer.h"
#include "ServerData.h"
#include "Geometry.h"
#include "Util.h"
#include "Packets.h"
#include "ServerUtil.h"
#include <cstdio>

using namespace server;
using namespace mat;
using namespace constants;
namespace server {
/////////////////////////////////
//////////// Player /////////////
/////////////////////////////////
Player::Player(int id, int sid, vec2 pos, vec2 dir, int hp)
   : PlayerBase(id, pos), sid(sid), dir(dir), moving(false), 
   hp(hp), rupees(0), exp(0), pvp(false), shotThisFrame(false)
{
   
}

Player::Player(pack::Packet &serialized)
   : PlayerBase(0, vec2())
{
   deserialize(serialized);
}

void Player::move(vec2 pos, vec2 dir, bool moving)
{
   getOM().move(this, pos);
   this->dir = dir;
   this->moving = moving;
}

void Player::takeDamage(int damage)
{
   hp = max(0, hp-damage);
}

void Player::gainExp(int exp)
{
   this->exp += exp;
}

void Player::gainRupees(int rupees)
{
   this->rupees += rupees;
}

void Player::deserialize(pack::Packet &serialized)
{
   if(serialized.type == PacketType::serialPlayer) {
      int ipvp, ialive, imoving;
      serialized.data.readInt(id).readInt(sid).readInt(hp).readInt(exp).readInt(rupees)
         .readFloat(pos.x).readFloat(pos.y).readFloat(dir.x).readFloat(dir.y)
         .readInt(imoving).readInt(ialive).readInt(ipvp).reset();
      pvp = ipvp != 0; //warningless cast to bool
      alive = ialive != 0;
      moving = imoving != 0;
   } 
   else
      printf("Error: Deserializing Player with incorrect packet.\n");
}

pack::Packet Player::serialize() const
{
   pack::Packet p(PacketType::serialPlayer);
   int ipvp = (int)pvp;
   int ialive = (int)alive;
   int imoving = (int)moving;
   p.data.writeUInt(id).writeInt(sid).writeInt(hp).writeInt(exp).writeInt(rupees)
      .writeFloat(pos.x).writeFloat(pos.y).writeFloat(dir.x).writeFloat(dir.y)
      .writeInt(imoving).writeInt(ialive).writeInt(ipvp);
   return p;
}

pack::Packet Player::cserialize() const
{
   pack::Initialize ini;
   ini.id = id;
   ini.hp = hp;
   ini.dir = dir;
   ini.pos = pos;
   ini.subType = 0;
   ini.type = getType();
   return ini.makePacket();
}

void Player::gainHp(int hp)
{
   this->hp = min(this->hp + hp, playerMaxHp);
}

/////////////////////////////////
//////////// Missile ////////////
/////////////////////////////////
Missile::Missile(int id, int sid, int owned, mat::vec2 pos, mat::vec2 dir, int type)
   : MissileBase(id, type, pos), sid(sid), owned(owned), dir(dir)
{
   spawnTime = getTicks();
   this->dir = dir;
   if (this->dir.length() > 0.0f)
      this->dir.normalize();
   else
      this->dir = vec2(1,0);
}

Missile::Missile(pack::Packet &serialized)
   : MissileBase(0, 0, vec2())
{
   deserialize(serialized);
}

void Missile::update()
{
   move(pos = pos + dir * projectileSpeed * getDt(), dir);
}

int Missile::getDamage() const
{
   return rand()%(missileDamageMax-missileDamageMin) + missileDamageMin;
}

void Missile::deserialize(pack::Packet &serialized)
{
   //printf("Error Missile::deserialize untested - update with members!");
   if(serialized.type == PacketType::serialMissile) {
      serialized.data.readInt(id).readInt(sid).readInt(owned).readInt(type).readInt(spawnTime)
         .readFloat(pos.x).readFloat(pos.y).readFloat(dir.x).readFloat(dir.y)
         .reset();
   } 
   else
      printf("Error: Deserializing Missile with incorrect packet.\n");
}

pack::Packet Missile::cserialize() const
{
   pack::Initialize ini;
   ini.id = id;
   ini.hp = 0;
   ini.dir = dir;
   ini.pos = pos;
   ini.subType = type;
   ini.type = getType();
   return ini.makePacket();
}

pack::Packet Missile::serialize() const
{
   //printf("Error Missile::serialize untested - update with members!");
   pack::Packet p(PacketType::serialMissile);
   p.data.writeInt(id).writeInt(sid).writeInt(owned).writeInt(type).writeInt(spawnTime)
      .writeFloat(pos.x).writeFloat(pos.y).writeFloat(dir.x).writeFloat(dir.y);
   return p;
}

void Missile::move(vec2 pos, vec2 dir)
{
   if(getOM().contains(id))
      getOM().move(this, pos);
   else
      getSOM().move(this, pos);
   this->pos = pos;
   this->dir = dir;
}

/////////////////////////////////
/////////////// NPC /////////////
/////////////////////////////////
NPC::NPC(int id, int sid, int hp, vec2 pos, vec2 dir, int type)
   : NPCBase(id, type, pos), sid(sid), hp(hp), dir(dir),
   aiType(AIType::Stopped), aiTicks(0), attackId(0), initPos(pos), 
   moving(false), nextMissileTicks(0)
{

}

NPC::NPC(pack::Packet &serialized)
   : NPCBase(0, 0, vec2())
{
   deserialize(serialized);
}

int NPC::getAttackDelay() const
{
   switch(type) {
      case NPCType::Fairy:
      case NPCType::Bird:
      case NPCType::Squirrel: 
      case NPCType::Chicken:
         return 800;
         break; //unreachable
      case NPCType::Bush:
      case NPCType::Bat:
      case NPCType::Thief:
      case NPCType::Cactus:
      case NPCType::Goblin:
      case NPCType::Princess:
      case NPCType::Vulture:
         return 700;
         break;
      case NPCType::Cyclops:
      case NPCType::Skeleton:
      case NPCType::Wizard:
         return 600;
         break;
      case NPCType::BigFairy:
      case NPCType::Ganon:
         return 500;
         break;
      default:
         printf("Error NPC::getAttackDelay() - unknown NPC type %d\n", type);
   }
   return 0;
}

int NPC::getExp()
{
   int exp = 0;
   switch(type) {
      case NPCType::Fairy:
      case NPCType::Bird:
      case NPCType::Squirrel: 
      case NPCType::Chicken:
         exp = 1;
         break;
      case NPCType::Bush:
      case NPCType::Bat:
      case NPCType::Thief:
      case NPCType::Cactus:
      case NPCType::Goblin:
      case NPCType::Princess:
      case NPCType::Vulture:
         exp = 2;
         break;
      case NPCType::Cyclops:
      case NPCType::Skeleton:
      case NPCType::Wizard:
         exp = 4;
         break;
      case NPCType::BigFairy:
      case NPCType::Ganon:
         exp = 10;
         break;
      default:
         printf("Error NPC::getExp() - unknown NPC type %d\n", type);
   }
   return exp;
}

int NPC::getLoot()
{
   int roll = rand() % 100;
   int loot = -1;
   switch(type) {
      case NPCType::Fairy:
      case NPCType::Bird:
      case NPCType::Squirrel: 
      case NPCType::Chicken:
         loot = roll < 30 ? ItemType::GreenRupee :
            roll < 55 ? ItemType::Heart : 
            -1;
         break;
      case NPCType::Bush:
      case NPCType::Bat:
      case NPCType::Thief:
      case NPCType::Cactus:
      case NPCType::Goblin:
      case NPCType::Princess:
      case NPCType::Vulture:
         loot = roll < 40 ? ItemType::GreenRupee :
            roll < 50 ? ItemType::BlueRupee :
            roll < 80 ? ItemType::Heart : 
            -1;
         break;
      case NPCType::Cyclops:
      case NPCType::Skeleton:
      case NPCType::Wizard:
         loot = roll < 20 ? ItemType::GreenRupee :
            roll < 30 ? ItemType::BlueRupee :
            roll < 50 ? ItemType::RedRupee :
            roll < 90 ? ItemType::Heart :
            -1;
         break;
      case NPCType::BigFairy:
      case NPCType::Ganon:
         loot = roll < 20 ? ItemType::BlueRupee :
            roll < 50 ? ItemType::RedRupee :
            roll < 95 ? ItemType::Heart :
            -1;
         break;
      default:
         printf("Error NPC::getLoot() - unknown NPC type %d\n", type);
   }
   return loot;
}

void NPC::update() 
{
   Player *p = 0;
   moving = false;
   if(mat::dist(pos, initPos) > maxNpcMoveDist) {
      aiType = AIType::Walking;
      dir = mat::to(pos, initPos);
      dir.normalize();
      aiTicks = getTicks() + rand() % 1000;
      if(!npcStaysAggroWhenInRange)
         return;
   }

   if(aiType == AIType::Attacking 
         && (!p || !getOM().contains(attackId, ObjectType::Player)
         || !getSOM().contains(attackId, ObjectType::Player))) {
      attackId = 0;
      aiType = AIType::Stopped;
      return;
   }
   else {
      Geometry aggroCircle(Circle(pos, npcAggroRange));
      std::vector<PlayerBase *> closePlayers;
      getOM().collidingPlayers(aggroCircle, pos, closePlayers);
      getSOM().collidingPlayers(aggroCircle, pos, closePlayers);
      if(closePlayers.size() > 0) {
         aiType = AIType::Attacking;
         p = static_cast<Player *>(closePlayers[0]);
         attackId = p->getId();
         float dist = mat::dist(p->pos, pos);
         for(unsigned i = 1; i < closePlayers.size(); i++) {
            float dist2 = mat::dist(closePlayers[i]->pos, pos);
            if(dist2 < dist) {
               p = static_cast<Player *>(closePlayers[i]);
               attackId = p->getId();
            }
         }
      }
   }
   if(aiType == AIType::Attacking) {
      vec2 newDir = mat::to(pos, p->pos);
      if(newDir.length() > 0.0f) {
         newDir.normalize();
         dir = newDir;
         vec2 newPos = pos + dir * getDt() * npcAttackSpeed;
         if(mat::dist(newPos, p->pos) > attackRange) {
            move(newPos, newDir, true);
         }
      }
      if(getTicks() > nextMissileTicks) {
         nextMissileTicks = getTicks() + getAttackDelay();
         Missile *m = new Missile(newId(), id, sid, pos, 
            mat::to(this->pos, p->pos), 
            MissileType::Arrow);
         getGS().createMissile(m);
      }
   }
   else {
      gainHp(npcOutOfCombatHpPerTick);
      if(aiType == AIType::Walking) {
         move(pos + dir * getDt() * npcWalkSpeed, dir, true);
      }
      if(aiTicks - getTicks() <= 0
            && (aiType == AIType::Stopped || aiType == AIType::Walking)) {
         aiType = (rand() % 100) < 30 ? AIType::Stopped : AIType::Walking;
         aiTicks = getTicks() + rand() % 1000 + 300;
         if(aiType == AIType::Walking) {
            //float angle = ((rand() % 360) / 180.0f) * (float) PI;
            float angle = (float)((rand() /180.0f) * PI);
            dir = vec2(cos(angle), sin(angle));
            dir.normalize();
         }
      }
   }
}

void NPC::move(vec2 pos, vec2 dir, bool moving)
{
   getOM().move(this, pos);
   this->dir = dir;
   this->moving = moving;
}

void NPC::deserialize(pack::Packet &serialized)
{
   //printf("Error NPC::deserialize untested - update with members!");
   if(serialized.type == PacketType::serialNPC) {
      int imoving;
      serialized.data.readInt(id).readInt(sid).readInt(hp).readInt(type)
         .readInt(aiTicks).readInt(aiType).readInt(attackId)
         .readFloat(pos.x).readFloat(pos.y).readFloat(dir.x).readFloat(dir.y)
         .readFloat(initPos.x).readFloat(initPos.y).readInt(nextMissileTicks)
         .readInt(imoving).reset();
      moving = imoving != 0; //warningless cast to bool
   } 
   else
      printf("Error: Deserializing NPC with incorrect packet.\n");
}

pack::Packet NPC::serialize() const
{
   //printf("Error NPC::serialize untested - update with members!");
   pack::Packet p(PacketType::serialNPC);
   int imoving = (int)moving;
   p.data.writeInt(id).writeInt(sid).writeInt(hp).writeInt(type)
      .writeInt(aiTicks).writeInt(aiType).writeInt(attackId)
      .writeFloat(pos.x).writeFloat(pos.y).writeFloat(dir.x).writeFloat(dir.y)
      .writeFloat(initPos.x).writeFloat(initPos.y).writeInt(nextMissileTicks)
      .writeInt(imoving);
   return p;
}

pack::Packet NPC::cserialize() const
{
   pack::Initialize ini;
   ini.id = id;
   ini.hp = hp;
   ini.dir = dir;
   ini.pos = pos;
   ini.subType = type;
   ini.type = getType();
   return ini.makePacket();;
}

void NPC::gainHp(int hp)
{
   this->hp = this->hp + hp;
   util::clamp(this->hp, 0, npcMaxHp);
}

void NPC::takeDamage(int damage)
{
   hp = max(0, hp-damage);
}


/////////////////////////////////
////////////// Item /////////////
/////////////////////////////////
Item::Item(int id, int sid, vec2 pos, int type)
   : ItemBase(id, type, pos), sid(sid)
{

}

Item::Item(pack::Packet &serialized)
   : ItemBase(0, 0, vec2()), sid(0)
{
   deserialize(serialized);
}

void Item::move(vec2 pos)
{
   getOM().move(static_cast<ItemBase *>(this), pos);
}

bool Item::isCollidable() const
{
   switch (type) {
      case ItemType::GreenRupee:
      case ItemType::RedRupee:
      case ItemType::BlueRupee:
      case ItemType::Explosion:
      case ItemType::Stump:
      case ItemType::Heart:
         break;
      default:
         printf("Error Item::isCollidable - Unknown item type %d\n", type);
   }
   return type == ItemType::Stump;
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

void Item::deserialize(pack::Packet &serialized)
{
   //printf("Error Item::deserialize untested - update with members!");
   if(serialized.type == PacketType::serialItem) {
      serialized.data.readInt(id).readInt(sid).readInt(type)
         .readFloat(pos.x).readFloat(pos.y)
         .reset();
   } 
   else
      printf("Error: Deserializing Item with incorrect packet.\n");
}

pack::Packet Item::serialize() const
{
   //printf("Error Item::serialize untested - update with members!");
   pack::Packet p(PacketType::serialItem);
   p.data.writeInt(id).writeInt(sid).writeInt(type)
      .writeFloat(pos.x).writeFloat(pos.y);
   return p;
}

pack::Packet Item::cserialize() const
{
   pack::Initialize ini;
   ini.id = id;
   ini.hp = 0;
   //ini.dir = vec2()
   ini.pos = pos;
   ini.subType = type;
   ini.type = getType();
   return ini.makePacket();;
}

//////////////////////////////////////////
////////////// Object Holder /////////////
//////////////////////////////////////////
Player *ObjectHolder::getPlayer(int id) const
{
   return static_cast<Player *>(ObjectManager::getPlayer(id));
}

NPC *ObjectHolder::getNPC(int id) const
{
   return static_cast<NPC *>(ObjectManager::getNPC(id));
}

Item *ObjectHolder::getItem(int id) const
{
   return static_cast<Item *>(ObjectManager::getItem(id));
}

Missile *ObjectHolder::getMissile(int id) const
{
   return static_cast<Missile *>(ObjectManager::getMissile(id));
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

pack::Packet ObjectHolder::getSerialized(int id) const
{
   return getSerializable(id)->serialize();
}

pack::Packet ObjectHolder::getCSerialized(int id) const
{
   return getSerializable(id)->cserialize();
}

Serializable *ObjectHolder::getSerializable(int id) const
{
   ObjectBase *obj = static_cast<ObjectBase *>(get(id));
   Serializable *ser = 0;
   switch(obj->getType()) {
      case ObjectType::Player:
         ser = static_cast<Serializable *>(static_cast<Player *>(obj));
         break;
      case ObjectType::NPC:
         ser = static_cast<Serializable *>(static_cast<NPC *>(obj));
         break;
      case ObjectType::Item:
         ser = static_cast<Serializable *>(static_cast<Item *>(obj));
         break;
      case ObjectType::Missile:
         ser = static_cast<Serializable *>(static_cast<Missile *>(obj));
         break;
      default:
         printf("Error: getSerialized Unknown type %d\n", obj->getType());
   }
   return ser;
}

} // end server namespace
