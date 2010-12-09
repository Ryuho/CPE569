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
   : PlayerBase(id, pos), dir(dir), moving(false), alive(true), hp(hp),
   rupees(0), exp(0), pvp(false)
{
   lastUpdate = getTicks();
}

Player::Player(pack::Initialize &ini)
   : PlayerBase(-1, vec2()), alive(true)
{
   deserialize(ini);
}

Player::Player(pack::Packet &p) 
   : PlayerBase(-1, vec2()), alive(true)
{
   deserialize(p);
}

void Player::deserialize(pack::Initialize &ini)
{
   this->id = ini.id;
   this->pos = ini.pos;
   this->dir = ini.dir;
   this->hp = ini.hp;
   //this->type = ini.subType
   //this->pvp = ini.pvp;
}

void Player::deserialize(pack::Packet &p)
{
   p.data.setCursor(0);
   int _type, __type;

   p.data.readInt(id).readInt(_type).readInt(__type)
      .readFloat(pos.x).readFloat(pos.y).readFloat(dir.x)
      .readFloat(dir.y).readInt(hp).reset();
}

void Player::move(vec2 pos, vec2 dir, bool moving)
{
   lastUpdate = getTicks();
   if (moving && !this->moving)
      animStart = getTicks();
   this->pos = pos;
   this->dir = dir;
   if(this->dir.length() > 0.0)
      this->dir.normalize();
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
   : MissileBase(id, type, pos), alive(true)
{
   lastUpdate = getTicks();
   this->dir = dir;
   if(this->dir.length() > 0.0)
      this->dir.normalize();
}

Missile::Missile(pack::Packet &p)
   : MissileBase(-1, -1, vec2()), alive(true)
{
   deserialize(p);
}

Missile::Missile(pack::Initialize &ini)
   : MissileBase(-1, -1, vec2()), alive(true)
{
   deserialize(ini);
}

void Missile::deserialize(pack::Packet &p)
{
   p.data.setCursor(0);
   int hp, _type;

   p.data.readInt(id).readInt(_type).readInt(type)
      .readFloat(pos.x).readFloat(pos.y).readFloat(dir.x)
      .readFloat(dir.y).readInt(hp).reset();
}

void Missile::deserialize(pack::Initialize &ini)
{
   this->id = ini.id;
   this->pos = ini.pos;
   this->dir = ini.dir;
   //this->hp = ini.hp;
   this->type = ini.subType;
   //this->pvp = ini.pvp;
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
    this->lastUpdate = getTicks();
    initGraphics();
}

Item::Item(pack::Packet &p)
   : ItemBase(-1, -1, vec2()), alive(true)
{
   deserialize(p);
}

Item::Item(pack::Initialize &ini)
   : ItemBase(-1, -1, vec2()), alive(true)
{
   deserialize(ini);
}

void Item::deserialize(pack::Initialize &ini)
{
   this->id = ini.id;
   this->pos = ini.pos;
   //this->dir = ini.dir;
   //this->hp = ini.hp;
   this->type = ini.subType;
   //this->pvp = ini.pvp;
}

void Item::deserialize(pack::Packet &p)
{
   p.data.setCursor(0);
   int _type, hp;
   vec2 dir;

   p.data.readInt(id).readInt(_type).readInt(type)
      .readFloat(pos.x).readFloat(pos.y).readFloat(dir.x)
      .readFloat(dir.y).readInt(hp).reset();
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
   : NPCBase(id, type, pos), dir(dir), moving(moving), hp(hp), alive(true)
{
   initGraphics();
   this->lastUpdate = getTicks();
}

NPC::NPC(pack::Packet &p)
   : NPCBase(-1, -1, vec2()), alive(true)
{
   deserialize(p);
}


NPC::NPC(pack::Initialize &ini)
   : NPCBase(-1, -1, vec2()), alive(true)
{
   deserialize(ini);
}

void NPC::deserialize(pack::Initialize &ini)
{
   this->id = ini.id;
   this->pos = ini.pos;
   this->dir = ini.dir;
   this->hp = ini.hp;
   this->type = ini.subType;
   //this->pvp = ini.pvp;
}

void NPC::deserialize(pack::Packet &p)
{
   p.data.setCursor(0);
   int _type;

   p.data.readInt(id).readInt(_type).readInt(type)
      .readFloat(pos.x).readFloat(pos.y).readFloat(dir.x)
      .readFloat(dir.y).readInt(hp).reset();
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
   if(this->dir.length() > 0.0)
      this->dir.normalize();
   if(!this->moving && moving) {
      anim->animStart = getTicks();
   }
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

ObjectHolder::ObjectHolder() : ObjectManager()
{
   const float width(constants::worldWidth/2);
   const float height(constants::worldHeight/2);
   corners[Direction::Up] = vec2(-width, height);
   corners[Direction::Right] = vec2(width, height);
   corners[Direction::Down] = vec2(width, -height);
   corners[Direction::Left] = vec2(-width, -height);
}

void ObjectHolder::drawAll(bool checkNoDraw)
{
   int ticks = getTicks();

   for(unsigned i = 0; i < playerCount(); i++) {
      Player &obj = *static_cast<Player *>(get(ObjectType::Player, i));
      if(!checkNoDraw || ticks - obj.lastUpdate < noDrawTicks) {
         obj.draw();
      }
   }
   for(unsigned i = 0; i < npcCount(); i++) {
      NPC &obj = *static_cast<NPC *>(get(ObjectType::NPC, i));
      if(!checkNoDraw || ticks - obj.lastUpdate < noDrawTicks) {
         obj.draw();
      }
   }
   for(unsigned i = 0; i < itemCount(); i++) {
      Item &obj = *static_cast<Item *>(get(ObjectType::Item, i));
      obj.draw();
   }
   for(unsigned i = 0; i < missileCount(); i++) {
      Missile &obj = *static_cast<Missile *>(get(ObjectType::Missile, i));
      obj.draw();
   }

   for(unsigned i = 0; i < 4; i++) {
      for(unsigned j = 0; j < border[i].size(); j++) {
         border[i][j].draw();
      }
   }
}

void ObjectHolder::drawAll(vec2 pos, bool checkNoDraw)
{
   int ticks = getTicks();
   Geometry aoi(Circle(pos, constants::areaOfInfluenceRadius));

   std::vector<PlayerBase *> players;
   collidingPlayers(aoi, pos, players);
   for (unsigned i = 0; i < players.size(); i++) {
      Player &obj = *static_cast<Player *>(players[i]);
      if(!checkNoDraw || ticks - obj.lastUpdate < noDrawTicks) {
         obj.draw();
      }
   }
   std::vector<NPCBase *> npcs;
   collidingNPCs(aoi, pos, npcs);
   for (unsigned i = 0; i < npcs.size(); i++) {
      NPC &obj = *static_cast<NPC *>(npcs[i]);
      if(!checkNoDraw || ticks - obj.lastUpdate < noDrawTicks) {
         obj.draw();
      }
   }
   std::vector<ItemBase *> items;
   collidingItems(aoi, pos, items);
   for (unsigned i = 0; i < items.size(); i++) {
      Item &obj = *static_cast<Item *>(items[i]);
      obj.draw();
   }
   std::vector<MissileBase *> missiles;
   collidingMissiles(aoi, pos, missiles);
   for (unsigned i = 0; i < missiles.size(); i++) {
      Missile &obj = *static_cast<Missile *>(missiles[i]);
      obj.draw();
   }

   for(unsigned i = 0; i < 4; i++) {
      Geometry lineBorder(Plane(corners[i], corners[(i+1)%4], 1.0f));
      if(aoi.collision(lineBorder)) {
         for(unsigned j = 0; j < border[i].size(); j++) {
            border[i][j].draw();
         }
      }
   }
}
