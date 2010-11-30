#include "ServerData.h"
#include "GameServer.h"
#include "Geometry.h"
#include "Util.h"
#include <cstdio>

namespace server {
using namespace mat;
using namespace constants;

// Player

Player::Player(int id, int sid, vec2 pos, vec2 dir, int hp)
   : id(id), sid(sid), pos(pos), dir(dir), moving(false), hp(hp), 
   rupees(0), exp(0), pvp(false)
{
   
}

void Player::move(vec2 pos, vec2 dir, bool moving)
{
   //this->pos = pos; Error to do this
   //omMoveTemplate will get the wrong regions
   //Even changing it after om.move() will be wrong 
   //since if the move function doesn't like the positon

   getOM().move(this, pos);
   //Unreferenced in om.move() so okay to update anywhere
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

Geometry Player::getGeom() const
{
   return Circle(pos, (float)playerRadius);
}

int Player::getObjectType() const
{
   return ObjectType::Player;
}

Player::Player(pack::Packet &serialized)
{
   deserialize(serialized);
}

void Player::deserialize(pack::Packet &serialized)
{
   //printf("Error Player::deserialize untested - update with members!");
   if(serialized.type == pack::serialPlayer) {
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
   //printf("Error Player::serialize untested - update with members!");
   pack::Packet p(pack::serialPlayer);
   int ipvp = (int)pvp;
   int ialive = (int)alive;
   int imoving = (int)moving;
   p.data.writeInt(id).writeInt(sid).writeInt(hp).writeInt(exp).writeInt(rupees)
      .writeFloat(pos.x).writeFloat(pos.y).writeFloat(dir.x).writeFloat(dir.y)
      .writeInt(imoving).writeInt(ialive).writeInt(ipvp);
   return p;
}

void Player::gainHp(int hp)
{
   this->hp = this->hp + hp;
   util::clamp(this->hp, 0, playerMaxHp);
}


// Missile

Missile::Missile(int id, int sid, int owned, mat::vec2 pos, mat::vec2 dir, int type)
   : id(id), sid(sid), owned(owned), pos(pos), dir(dir), type(type)
{
   spawnTime = getTicks();
   this->dir = dir;
   if (this->dir.length() > 0.0f)
      this->dir = normalize(this->dir);
   else
      this->dir = vec2(1,0);
}

void Missile::update()
{
   move(pos + dir * projectileSpeed * getDt(), dir);
}

Geometry Missile::getGeom() const
{
   return Circle(pos, arrowRadius);
}

int Missile::getDamage() const
{
   return rand()%6 + 5;
}

int Missile::getObjectType() const
{
   return ObjectType::Missile;
}

Missile::Missile(pack::Packet &serialized)
{
   deserialize(serialized);
}

void Missile::deserialize(pack::Packet &serialized)
{
   //printf("Error Missile::deserialize untested - update with members!");
   if(serialized.type == pack::serialMissile) {
      serialized.data.readInt(id).readInt(sid).readInt(owned).readInt(type).readInt(spawnTime)
         .readFloat(pos.x).readFloat(pos.y).readFloat(dir.x).readFloat(dir.y)
         .reset();
   } 
   else
      printf("Error: Deserializing Missile with incorrect packet.\n");
}

pack::Packet Missile::serialize() const
{
   //printf("Error Missile::serialize untested - update with members!");
   pack::Packet p(pack::serialMissile);
   p.data.writeInt(id).writeInt(sid).writeInt(owned).writeInt(type).writeInt(spawnTime)
      .writeFloat(pos.x).writeFloat(pos.y).writeFloat(dir.x).writeFloat(dir.y);
   return p;
}

void Missile::move(vec2 pos, vec2 dir)
{
   //this->pos = pos; Error to do this
   //omMoveTemplate will get the wrong regions
   //Even changing it after om.move() will be wrong 
   //since if the move function doesn't like the positon

   getOM().move(this, pos);
   //Unreferenced in om.move() so okay to update anywhere
   this->dir = dir;
}

// NPC

NPC::NPC(int id, int sid, int hp, vec2 pos, vec2 dir, int type)
   : id(id), sid(sid), hp(hp), pos(pos), dir(dir), type(type), aiType(AIType::Stopped),
   aiTicks(0), attackId(0), initPos(pos), moving(false), nextMissileTicks(0)
{

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
   this->moving = false;
   if(mat::dist(pos, initPos) > maxNpcMoveDist) {
      aiType = AIType::Walking;
      dir = mat::to(pos, initPos);
      dir.normalize();
      aiTicks = getTicks() + rand() % 1000;
   }

   Geometry aggroCircle(Circle(pos, npcAggroRange));
   std::vector<Player *> closePlayers 
      = getOM().collidingPlayers(aggroCircle, pos);
   if(closePlayers.size() > 0) {
      aiType = AIType::Attacking;
      p = closePlayers[0];
      attackId = p->id;
      float dist = mat::dist(p->pos, pos);
      for(unsigned i = 1; i < closePlayers.size(); i++) {
         float dist2 = mat::dist(closePlayers[i]->pos, pos);
         if(dist2 < dist) {
            p = closePlayers[i];
            attackId = p->id;
         }
      }
   }
   if(aiType == AIType::Attacking 
         && (!getOM().check(attackId, ObjectType::Player) || !p)) {
      attackId = 0;
      aiType = AIType::Stopped;
      return;
   }
   if(aiTicks - getTicks() <= 0
         && (aiType == AIType::Stopped || aiType == AIType::Walking)) {
      aiType = (rand() % 100) < 30 ? AIType::Stopped : AIType::Walking;
      aiTicks = getTicks() + rand() % 1000 + 300;
      if(aiType == AIType::Walking) {
         float angle = ((rand() % 360) / 180.0f) * (float) PI;
         dir = vec2(cos(angle), sin(angle));
         dir.normalize();
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
            mat::to(this->pos, getOM().getPlayer(attackId)->pos), MissileType::Arrow);
         getOM().add(m);
         getCM().clientBroadcast(pack::Initialize(m->id, 
            ObjectType::Missile, m->type, m->pos, 
            m->dir, 0));
      }
   }
   else {
      this->gainHp(npcOutOfCombatHpPerTick);
      if(aiType == AIType::Walking) {
         move(pos + dir * getDt() * npcWalkSpeed, dir, true);
      }
   }
}

float NPC::getRadius() const
{
   switch(type) {
      case NPCType::Fairy: //16x16
      case NPCType::Bat:
      case NPCType::Bird:
         return 16*1.5f;
         break;
      case NPCType::Thief: //16x32
      case NPCType::Squirrel: 
      case NPCType::Princess:
      case NPCType::Skeleton:
      case NPCType::Cactus:
      case NPCType::Wizard:
      case NPCType::Goblin:
         return 22*1.5f;
         break;
      case NPCType::Cyclops: //32x32
      case NPCType::Chicken:
      case NPCType::Vulture:
      case NPCType::Bush:
      case NPCType::BigFairy:
      case NPCType::Ganon:
         return 23.5f*1.5f;
         break;
      default:
         printf("Error NPC::getRadius() - unknown NPC type %d\n", type);
   }
   return 0.0f;
}

Geometry NPC::getGeom() const
{
   return Circle(pos, getRadius());
}

void NPC::takeDamage(int damage)
{
   hp = max(0, hp-damage);
}

void NPC::move(vec2 pos, vec2 dir, bool moving)
{
   //this->pos = pos; Error to do this
   //omMoveTemplate will get the wrong regions
   //Even changing it after om.move() will be wrong 
   //since if the move function doesn't like the positon

   getOM().move(this, pos);
   //Unreferenced in om.move() so okay to update anywhere
   this->dir = dir;
   this->moving = moving;
}

int NPC::getObjectType() const
{
   return ObjectType::NPC;
}

NPC::NPC(pack::Packet &serialized)
{
   deserialize(serialized);
}

void NPC::deserialize(pack::Packet &serialized)
{
   //printf("Error NPC::deserialize untested - update with members!");
   if(serialized.type == pack::serialNPC) {
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
   pack::Packet p(pack::serialNPC);
   int imoving = (int)moving;
   p.data.writeInt(id).writeInt(sid).writeInt(hp).writeInt(type)
      .writeInt(aiTicks).writeInt(aiType).writeInt(attackId)
      .writeFloat(pos.x).writeFloat(pos.y).writeFloat(dir.x).writeFloat(dir.y)
      .writeFloat(initPos.x).writeFloat(initPos.y).writeInt(nextMissileTicks)
      .writeInt(imoving);
   return p;
}

void NPC::gainHp(int hp)
{
   this->hp = this->hp + hp;
   util::clamp(this->hp, 0, npcMaxHp);
}

// Item

Item::Item(int id, int sid, vec2 pos, int type)
   : id(id), sid(sid),pos(pos), type(type)
{

}

void Item::move(vec2 pos)
{
   //this->pos = pos; Error to do this
   //omMoveTemplate will get the wrong regions
   //Even changing it after om.move() will be wrong 
   //since if the move function doesn't like the positon

   getOM().move(this, pos);
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

float Item::getRadius() const
{
   switch (type) {
      case ItemType::GreenRupee:
      case ItemType::RedRupee:
      case ItemType::BlueRupee:
         return 12*1.5f;
         break; //unreachable
      case ItemType::Explosion:
         return 55*1.5f;
         break;
      case ItemType::Stump:
         return 32*1.5f;
         break;
      case ItemType::Heart:
         return 13*1.5f;
         break;
      default:
         printf("Error Item::getGeom - Unknown item type %d\n", type);
   }
   return 0.0f;
}

Geometry Item::getGeom() const
{
   return Circle(pos, getRadius());
}

int Item::getObjectType() const
{
   return ObjectType::Item;
}

Item::Item(pack::Packet &serialized)
{
   deserialize(serialized);
}

void Item::deserialize(pack::Packet &serialized)
{
   //printf("Error Item::deserialize untested - update with members!");
   if(serialized.type == pack::serialItem) {
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
   pack::Packet p(pack::serialItem);
   p.data.writeInt(id).writeInt(sid).writeInt(type)
      .writeFloat(pos.x).writeFloat(pos.y);
   return p;
}

} // end server namespace
