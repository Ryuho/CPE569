#include "ServerData.h"
#include "GameServer.h"
#include "Geometry.h"
#include "Util.h"
#include <cstdio>

namespace server {
using namespace mat;
using namespace constants;

// Player

Player::Player(int id, vec2 pos, vec2 dir, int hp)
   : id(id), pos(pos), dir(dir), moving(false), hp(hp), rupees(0), exp(0)
{
   
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
   printf("Error Player::deserialize untested - update with members!");
   if(serialized.type == pack::serialPlayer) {
      serialized.data.readInt(id).readInt(hp).readInt(exp).readInt(rupees)
         .readFloat(pos.x).readFloat(pos.y).readFloat(dir.x).readFloat(dir.y)
         .readBit(moving).readBit(alive).reset();
   } 
   else
      printf("Error: Deserializing Player with incorrect packet.\n");
}

pack::Packet Player::serialize() const
{
   printf("Error Player::serialize untested - update with members!");
   pack::Packet p(pack::serialPlayer);
   p.data.writeInt(id).writeInt(hp).writeInt(exp).writeInt(rupees)
      .writeFloat(pos.x).writeFloat(pos.y).writeFloat(dir.x).writeFloat(dir.y)
      .writeBit(moving).writeBit(alive);
   return p;
}


// Missile

Missile::Missile(int id, int owned, mat::vec2 pos, mat::vec2 dir, int type)
   : id(id), owned(owned), pos(pos), dir(dir), type(type)
{
   spawnTime = getTicks();
   this->dir = dir;
   if (this->dir.length() > 0.0)
      this->dir = normalize(this->dir);
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
   printf("Error Missile::deserialize untested - update with members!");
   if(serialized.type == pack::serialMissile) {
      serialized.data.readInt(id).readInt(owned).readInt(type).readInt(spawnTime)
         .readFloat(pos.x).readFloat(pos.y).readFloat(dir.x).readFloat(dir.y)
         .reset();
   } 
   else
      printf("Error: Deserializing Missile with incorrect packet.\n");
}

pack::Packet Missile::serialize() const
{
   printf("Error Missile::serialize untested - update with members!");
   pack::Packet p(pack::serialMissile);
   p.data.writeInt(id).writeInt(owned).writeInt(type).writeInt(spawnTime)
      .writeFloat(pos.x).writeFloat(pos.y).writeFloat(dir.x).writeFloat(dir.y);
   return p;
}

void Missile::move(vec2 pos, vec2 dir)
{
   getOM().move(this, pos);
   this->pos = pos;
   this->dir = dir;
}

// NPC

NPC::NPC(int id, int hp, vec2 pos, vec2 dir, int type)
   : id(id), hp(hp), pos(pos), dir(dir), type(type), aiType(AIType::Stopped),
   aiTicks(0), attackId(0), initPos(pos), moving(false)
{

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
   for(unsigned i = 0; i < getOM().players.size(); i++) {
      if(mat::dist(getOM().players[i]->pos, pos) < npcAggroRange) {
         p = getOM().players[i];
         aiType = AIType::Attacking;
         attackId = p->id;
         //printf("%d attacking %d\n", id, attackId);
      }
   }
   if(aiType == AIType::Attacking) {
      if(!getOM().check(attackId, ObjectType::Player) 
            || mat::dist(getOM().getPlayer(attackId)->pos, pos) >= npcAggroRange) {
         attackId = 0;
         aiType = AIType::Stopped;
         return;
      }
   }
   if(aiTicks - getTicks() <= 0) {
      if(aiType == AIType::Stopped || aiType == AIType::Walking) {
         aiType = (rand() % 100) < 30 ? AIType::Stopped : AIType::Walking;
         aiTicks = getTicks() + rand() % 1000 + 300;
         if(aiType == AIType::Walking) {
            float angle = ((rand() % 360) / 180.0f) * PI;
            dir = vec2(cos(angle), sin(angle));
            dir.normalize();
         }
      }
   }
   if(aiType == AIType::Attacking) {
      vec2 newDir = mat::to(pos, p->pos);
      if(newDir.length() > 0.2f) {
         newDir.normalize();
         dir = newDir;
         vec2 newPos = pos + dir * getDt() * npcAttackSpeed;
         if(mat::dist(newPos, p->pos) > attackRange) {
            move(newPos, newDir, true);
         }
      }
   }
   else if(aiType == AIType::Walking) {
      move(pos + dir * getDt() * npcWalkSpeed, dir, true);
   }
}

Geometry NPC::getGeom() const
{
   switch(type) {
      case NPCType::Fairy: //16x16
      case NPCType::Bat:
      case NPCType::Bird:
         return Circle(pos, 16*1.5f);
         break;
      case NPCType::Thief: //16x32
      case NPCType::Squirrel: 
      case NPCType::Princess:
      case NPCType::Skeleton:
      case NPCType::Cactus:
      case NPCType::Wizard:
      case NPCType::Goblin:
         return Circle(pos, 22*1.5f);
         break;
      case NPCType::Cyclops: //32x32
      case NPCType::Chicken:
      case NPCType::Vulture:
      case NPCType::Bush:
      case NPCType::BigFairy:
      case NPCType::Ganon:
         return Circle(pos, 23.5f*1.5f);
         break;
      default:
         printf("Error NPC::getGeom() - unknown NPC type %d\n", type);
   }
   return Circle(pos, 0.0f);
}

void NPC::takeDamage(int damage)
{
   hp = max(0, hp-damage);
}

void NPC::move(vec2 pos, vec2 dir, bool moving)
{
   getOM().move(this, pos);
   this->pos = pos;
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
   printf("Error NPC::deserialize untested - update with members!");
   if(serialized.type == pack::serialNPC) {
      serialized.data.readInt(id).readInt(hp).readInt(type)
         .readInt(aiTicks).readInt(aiType).readInt(attackId)
         .readFloat(pos.x).readFloat(pos.y).readFloat(dir.x).readFloat(dir.y)
         .readFloat(initPos.x).readFloat(initPos.y).reset();
   } 
   else
      printf("Error: Deserializing NPC with incorrect packet.\n");
}

pack::Packet NPC::serialize() const
{
   printf("Error NPC::serialize untested - update with members!");
   pack::Packet p(pack::serialNPC);
   p.data.writeInt(id).writeInt(hp).writeInt(type)
      .writeInt(aiTicks).writeInt(aiType).writeInt(attackId)
      .writeFloat(pos.x).writeFloat(pos.y).writeFloat(dir.x).writeFloat(dir.y)
      .writeFloat(initPos.x).writeFloat(initPos.y);
   return p;
}

// Item

Item::Item(int id, vec2 pos, int type)
   : id(id), pos(pos), type(type)
{

}

void Item::move(vec2 pos)
{
   getOM().move(this, pos);
   this->pos = pos;
}

Geometry Item::getGeom() const
{
   switch (type) {
      case ItemType::GreenRupee:
      case ItemType::RedRupee:
      case ItemType::BlueRupee:
         return Circle(pos, 12*1.5f);
         break; //unreachable
      case ItemType::Explosion:
         return Circle(pos, 55*1.5f);
         break;
      case ItemType::Stump:
         return Circle(pos, 32*1.5f);
         break;
      case ItemType::Heart:
         return Circle(pos, 13*1.5f);
         break;
      default:
         printf("Error Item::getGeom - Unknown item type %d\n", type);
   }
   return Circle(pos, 0.0f);
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
   printf("Error Item::deserialize untested - update with members!");
   if(serialized.type == pack::serialItem) {
      serialized.data.readInt(id).readInt(type)
         .readFloat(pos.x).readFloat(pos.y)
         .reset();
   } 
   else
      printf("Error: Deserializing Item with incorrect packet.\n");
}

pack::Packet Item::serialize() const
{
   printf("Error Item::serialize untested - update with members!");
   pack::Packet p(pack::serialItem);
   p.data.writeInt(id).writeInt(type)
      .writeFloat(pos.x).writeFloat(pos.y);
   return p;
}

} // end server namespace
