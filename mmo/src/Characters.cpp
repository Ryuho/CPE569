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
   : MissileBase(id, type, pos), alive(true)
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
   : ItemBase(id, type, pos)
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
   : NPCBase(id, type, pos), dir(dir), moving(moving), hp(hp), alive(true)
{
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

ObjectHolder::ObjectHolder() : ObjectManager()
{
   const float width(constants::worldWidth/2);
   const float height(constants::worldHeight/2);
   //const float width = constants::worldWidth;
   //const float height = constants::worldHeight;
   corners[Direction::Up] = vec2(-width, height);
   corners[Direction::Right] = vec2(width, height);
   corners[Direction::Down] = vec2(width, -height);
   corners[Direction::Left] = vec2(-width, -height);
}

void ObjectHolder::drawAll(vec2 pos)
{
   int ticks = getTicks();
   Geometry aoi(Circle(pos, constants::areaOfInfluenceRadius));

   std::vector<PlayerBase *> players;
   collidingPlayers(aoi, pos, players);
   for (unsigned i = 0; i < players.size(); i++) {
      Player &obj = *static_cast<Player *>(players[i]);
      if(ticks - obj.lastUpdate < noDrawTicks) {
         obj.draw();
      }
   }
   std::vector<NPCBase *> npcs;
   collidingNPCs(aoi, pos, npcs);
   for (unsigned i = 0; i < npcs.size(); i++) {
      NPC &obj = *static_cast<NPC *>(npcs[i]);
      if(ticks - obj.lastUpdate < noDrawTicks) {
         obj.draw();
      }
      else if(mat::dist(obj.pos, pos) < constants::areaOfInfluenceRadius/2)
         printf("Error drawAll() %d\n", obj.getId());
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
