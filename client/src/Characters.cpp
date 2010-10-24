#include "Characters.h"

namespace {
   Texture spriteTex;
   Sprite sprites32;
   Sprite sprites64;
   Sprite sprites16x32;
   Sprite sprites16;
   Sprite sprites8;

   int spriteZoom = 4;

   bool initalized = false;

   Animation thiefAnim;
   Animation princessAnim;
   Animation fairyAnim;
   Animation skeletonAnim;
   Animation cyclopsAnim;
   Animation batAnim;
   Animation birdAnim;
   Animation squirrelAnim;
   Animation chickenAnim;
   Animation vultureAnim;
   Animation bushAnim;
   Animation cactusAnim;
   Animation bigFairyAnim;
   Animation wizardAnim;
   Animation ganonAnim;
   Animation goblinAnim;

   Animation greenRupeeAnim;
   Animation redRupeeAnim;
   Animation blueRupeeAnim;
   Animation explosionAnim;
}


void initCharacterResources()
{
   if (!initalized) {
      TexOptions opt;
      opt.min_filter = GL_NEAREST;
      opt.mag_filter = GL_NEAREST;
      spriteTex = fromTGA("character.tga", opt);
      sprites32 = Sprite(spriteTex, 32, 32);
      sprites64 = Sprite(spriteTex, 64, 64);
      sprites16x32 = Sprite(spriteTex, 16, 32);
      sprites16 = Sprite(spriteTex, 16, 16);
      sprites8 = Sprite(spriteTex, 8, 8);

      //////////////////////////////////////////////
      /////////////// NPC Animations ///////////////
      //////////////////////////////////////////////
      thiefAnim.init(&sprites16x32, Animation::Normal, false);
      thiefAnim.speed = 175;
      thiefAnim.dirs[Animation::Up].push_back(vec2i(11, 6)); //up
      thiefAnim.dirs[Animation::Up].push_back(vec2i(0,  6));
      thiefAnim.dirs[Animation::Up].push_back(vec2i(1,  6));
      thiefAnim.dirs[Animation::Right].push_back(vec2i(6,  6)); //right
      thiefAnim.dirs[Animation::Right].push_back(vec2i(7,  6));
      thiefAnim.dirs[Animation::Right].push_back(vec2i(6,  6));
      thiefAnim.dirs[Animation::Down].push_back(vec2i(10, 6)); //down
      thiefAnim.dirs[Animation::Down].push_back(vec2i(4,  6));
      thiefAnim.dirs[Animation::Down].push_back(vec2i(5,  6));
      thiefAnim.dirs[Animation::Left].push_back(vec2i(3,  6)); //left
      thiefAnim.dirs[Animation::Left].push_back(vec2i(2,  6));
      thiefAnim.dirs[Animation::Left].push_back(vec2i(3,  6));

      princessAnim.init(&sprites16x32, Animation::Normal, false);
      princessAnim.speed = 200;
      princessAnim.dirs[Animation::Up].push_back(vec2i(18, 6)); //up
      princessAnim.dirs[Animation::Up].push_back(vec2i(19, 6));
      princessAnim.dirs[Animation::Up].push_back(vec2i(18, 6));
      princessAnim.dirs[Animation::Right].push_back(vec2i(16, 6)); //right
      princessAnim.dirs[Animation::Right].push_back(vec2i(17, 6));
      princessAnim.dirs[Animation::Right].push_back(vec2i(16, 6));
      princessAnim.dirs[Animation::Down].push_back(vec2i(14, 6)); //down
      princessAnim.dirs[Animation::Down].push_back(vec2i(15, 6));
      princessAnim.dirs[Animation::Down].push_back(vec2i(14, 6)); 
      princessAnim.dirs[Animation::Left].push_back(vec2i(20, 6)); //left
      princessAnim.dirs[Animation::Left].push_back(vec2i(21, 6));
      princessAnim.dirs[Animation::Left].push_back(vec2i(20, 6));

      fairyAnim.init(&sprites16, Animation::LeftRight, true);
      fairyAnim.dirs[Animation::Right].push_back(vec2i(22, 0)); //right
      fairyAnim.dirs[Animation::Right].push_back(vec2i(23, 0));
      fairyAnim.dirs[Animation::Left].push_back(vec2i(22, 1)); //left
      fairyAnim.dirs[Animation::Left].push_back(vec2i(23, 1));

      skeletonAnim.init(&sprites16x32, Animation::Normal, false);
      skeletonAnim.speed = 200;
      skeletonAnim.dirs[Animation::Up].push_back(vec2i(30, 0)); //up
      skeletonAnim.dirs[Animation::Up].push_back(vec2i(31, 0));
      skeletonAnim.dirs[Animation::Up].push_back(vec2i(30, 0));
      skeletonAnim.dirs[Animation::Right].push_back(vec2i(24, 0)); //right
      skeletonAnim.dirs[Animation::Right].push_back(vec2i(25, 0));
      skeletonAnim.dirs[Animation::Right].push_back(vec2i(24, 0));
      skeletonAnim.dirs[Animation::Down].push_back(vec2i(28, 0)); //down
      skeletonAnim.dirs[Animation::Down].push_back(vec2i(29, 0));
      skeletonAnim.dirs[Animation::Down].push_back(vec2i(28, 0)); 
      skeletonAnim.dirs[Animation::Left].push_back(vec2i(27, 0)); //left
      skeletonAnim.dirs[Animation::Left].push_back(vec2i(26, 0));
      skeletonAnim.dirs[Animation::Left].push_back(vec2i(27, 0));

      cyclopsAnim.init(&sprites32, Animation::Normal, false);
      cyclopsAnim.speed = 200;
      cyclopsAnim.dirs[Animation::Up].push_back(vec2i(12, 7)); //up
      cyclopsAnim.dirs[Animation::Up].push_back(vec2i(11, 7));
      cyclopsAnim.dirs[Animation::Up].push_back(vec2i(12, 7));
      cyclopsAnim.dirs[Animation::Up].push_back(vec2i(13, 7));
      cyclopsAnim.dirs[Animation::Up].push_back(vec2i(12, 7));
      cyclopsAnim.dirs[Animation::Right].push_back(vec2i(7,  7));  //right
      cyclopsAnim.dirs[Animation::Right].push_back(vec2i(6,  7));
      cyclopsAnim.dirs[Animation::Right].push_back(vec2i(7,  7));
      cyclopsAnim.dirs[Animation::Down].push_back(vec2i(4,  7)); //down
      cyclopsAnim.dirs[Animation::Down].push_back(vec2i(8,  7));
      cyclopsAnim.dirs[Animation::Down].push_back(vec2i(9,  7)); 
      cyclopsAnim.dirs[Animation::Down].push_back(vec2i(10, 7));
      cyclopsAnim.dirs[Animation::Down].push_back(vec2i(9,  7)); 
      cyclopsAnim.dirs[Animation::Left].push_back(vec2i(12, 8)); //left
      cyclopsAnim.dirs[Animation::Left].push_back(vec2i(13, 8));
      cyclopsAnim.dirs[Animation::Left].push_back(vec2i(12, 8));

      batAnim.init(&sprites16, Animation::Forward, true);
      batAnim.speed = 175;
      batAnim.dirs[Animation::Down].push_back(vec2i(24, 18)); //down
      batAnim.dirs[Animation::Down].push_back(vec2i(24, 19));

      birdAnim.init(&sprites16, Animation::LeftRight, true);
      birdAnim.speed = 150;
      birdAnim.dirs[Animation::Right].push_back(vec2i(0, 10)); //right
      birdAnim.dirs[Animation::Right].push_back(vec2i(1, 10));
      birdAnim.dirs[Animation::Left].push_back(vec2i(0, 11)); //left
      birdAnim.dirs[Animation::Left].push_back(vec2i(1, 11));

      squirrelAnim.init(&sprites16x32, Animation::LeftRight, false);
      squirrelAnim.speed = 150;
      squirrelAnim.dirs[Animation::Right].push_back(vec2i(2, 5)); //right
      squirrelAnim.dirs[Animation::Right].push_back(vec2i(3, 5));
      squirrelAnim.dirs[Animation::Right].push_back(vec2i(2, 5));
      squirrelAnim.dirs[Animation::Left].push_back(vec2i(4, 5)); //left
      squirrelAnim.dirs[Animation::Left].push_back(vec2i(5, 5));
      squirrelAnim.dirs[Animation::Left].push_back(vec2i(4, 5));

      chickenAnim.init(&sprites32, Animation::LeftRight, false);
      chickenAnim.speed = 175;
      chickenAnim.dirs[Animation::Right].push_back(vec2i(3, 5)); //right
      chickenAnim.dirs[Animation::Right].push_back(vec2i(4, 5));
      chickenAnim.dirs[Animation::Right].push_back(vec2i(5, 5));
      chickenAnim.dirs[Animation::Right].push_back(vec2i(3, 5));
      chickenAnim.dirs[Animation::Left].push_back(vec2i(6, 5)); //left
      chickenAnim.dirs[Animation::Left].push_back(vec2i(7, 5));
      chickenAnim.dirs[Animation::Left].push_back(vec2i(8, 5));
      chickenAnim.dirs[Animation::Left].push_back(vec2i(6, 5));

      vultureAnim.init(&sprites32, Animation::Forward, true);
      vultureAnim.dirs[Animation::Down].push_back(vec2i(9, 5)); //down
      vultureAnim.dirs[Animation::Down].push_back(vec2i(10, 5));
      vultureAnim.dirs[Animation::Down].push_back(vec2i(11, 5));
      vultureAnim.dirs[Animation::Down].push_back(vec2i(10, 5));

      bushAnim.init(&sprites32, Animation::Forward, false);
      bushAnim.speed = 300;
      bushAnim.dirs[Animation::Down].push_back(vec2i(12, 3)); //down
      bushAnim.dirs[Animation::Down].push_back(vec2i(11, 3));
      bushAnim.dirs[Animation::Down].push_back(vec2i(12, 3));
      bushAnim.dirs[Animation::Down].push_back(vec2i(13, 3));
      bushAnim.dirs[Animation::Down].push_back(vec2i(12, 3));

      cactusAnim.init(&sprites16x32, Animation::Forward, false);
      cactusAnim.speed = 300;
      cactusAnim.dirs[Animation::Down].push_back(vec2i(20, 0)); //down
      cactusAnim.dirs[Animation::Down].push_back(vec2i(19, 0));
      cactusAnim.dirs[Animation::Down].push_back(vec2i(20, 0));
      cactusAnim.dirs[Animation::Down].push_back(vec2i(21, 0));
      cactusAnim.dirs[Animation::Down].push_back(vec2i(20, 0));

      bigFairyAnim.init(&sprites32, Animation::Forward, true);
      bigFairyAnim.speed = 200;
      bigFairyAnim.dirs[Animation::Down].push_back(vec2i(11, 4)); //down
      bigFairyAnim.dirs[Animation::Down].push_back(vec2i(12, 4));
      bigFairyAnim.dirs[Animation::Down].push_back(vec2i(13, 4));
      bigFairyAnim.dirs[Animation::Down].push_back(vec2i(12, 4));

      wizardAnim.init(&sprites16x32, Animation::Forward, false);
      wizardAnim.speed = 400;
      wizardAnim.dirs[Animation::Down].push_back(vec2i(12, 6)); //down
      wizardAnim.dirs[Animation::Down].push_back(vec2i(13, 6));
      wizardAnim.dirs[Animation::Down].push_back(vec2i(12, 6));

      ganonAnim.init(&sprites32, Animation::Forward, true);
      ganonAnim.speed = 350;
      ganonAnim.dirs[Animation::Down].push_back(vec2i(9, 4)); //down
      ganonAnim.dirs[Animation::Down].push_back(vec2i(10, 4));

      goblinAnim.init(&sprites32, Animation::LeftRight, false);
      goblinAnim.speed = 250;
      goblinAnim.dirs[Animation::Right].push_back(vec2i(2, 7)); //right
      goblinAnim.dirs[Animation::Right].push_back(vec2i(3, 7));
      goblinAnim.dirs[Animation::Right].push_back(vec2i(2, 7));
      goblinAnim.dirs[Animation::Left].push_back(vec2i(1, 7)); //left
      goblinAnim.dirs[Animation::Left].push_back(vec2i(0, 7));
      goblinAnim.dirs[Animation::Left].push_back(vec2i(1, 7));

      ///////////////////////////////////////////////
      /////////////// Item Animations ///////////////
      ///////////////////////////////////////////////
      greenRupeeAnim.init(&sprites16, Animation::Forward, true);
      greenRupeeAnim.speed = 400;
      greenRupeeAnim.dirs[Animation::Down].push_back(vec2i(13,0));
      greenRupeeAnim.dirs[Animation::Down].push_back(vec2i(13,0));
      greenRupeeAnim.dirs[Animation::Down].push_back(vec2i(13,0));
      greenRupeeAnim.dirs[Animation::Down].push_back(vec2i(13,0));
      greenRupeeAnim.dirs[Animation::Down].push_back(vec2i(14,0));
      greenRupeeAnim.dirs[Animation::Down].push_back(vec2i(15,0));

      redRupeeAnim.init(&sprites16, Animation::Forward, true);
      redRupeeAnim.speed = 400;
      redRupeeAnim.dirs[Animation::Down].push_back(vec2i(16,0));
      redRupeeAnim.dirs[Animation::Down].push_back(vec2i(16,0));
      redRupeeAnim.dirs[Animation::Down].push_back(vec2i(16,0));
      redRupeeAnim.dirs[Animation::Down].push_back(vec2i(16,0));
      redRupeeAnim.dirs[Animation::Down].push_back(vec2i(17,0));
      redRupeeAnim.dirs[Animation::Down].push_back(vec2i(18,0));

      blueRupeeAnim.init(&sprites16, Animation::Forward, true);
      blueRupeeAnim.speed = 400;
      blueRupeeAnim.dirs[Animation::Down].push_back(vec2i(16,1));
      blueRupeeAnim.dirs[Animation::Down].push_back(vec2i(16,1));
      blueRupeeAnim.dirs[Animation::Down].push_back(vec2i(16,1));
      blueRupeeAnim.dirs[Animation::Down].push_back(vec2i(16,1));
      blueRupeeAnim.dirs[Animation::Down].push_back(vec2i(17,1));
      blueRupeeAnim.dirs[Animation::Down].push_back(vec2i(18,1));

      explosionAnim.init(&sprites64, Animation::Forward, true);
      explosionAnim.speed = 150;
      explosionAnim.dirs[Animation::Down].push_back(vec2i(7,2));
      explosionAnim.dirs[Animation::Down].push_back(vec2i(7,3));
      explosionAnim.dirs[Animation::Down].push_back(vec2i(7,4));
      explosionAnim.dirs[Animation::Down].push_back(vec2i(7,5));
      explosionAnim.dirs[Animation::Down].push_back(vec2i(7,6));
      explosionAnim.dirs[Animation::Down].push_back(vec2i(7,7));

      initalized = true;
   }
}

void Player::update(mat::vec2 pos, mat::vec2 dir, bool moving)
{
   if (!this->moving && moving) {
      animStart = SDL_GetTicks();
   }

   this->pos = pos;
   if (dir.length() > 0.0)
      this->dir = normalize(dir);
   else
      this->dir = vec2(0.0, 1.0);
   this->moving = moving;
}

void Player::draw()
{
   int frame, adir;

   if (moving)
      frame = ((SDL_GetTicks() - animStart) / 100) % 8;
   else
      frame = 0;

   if (dir.y > 0.8)
      adir = 2;
   else if (dir.y < -0.8)
      adir = 4;
   else if (dir.x > 0)
      adir = 3;
   else
      adir = 1;

   glPushMatrix();
   glTranslatef(pos.x, pos.y, 0.0);
   glScalef(spriteZoom, spriteZoom, 1.0);
   if (moving)
      sprites32.draw(frame, adir);
   else
      sprites32.draw(adir-1, 0);
   glPopMatrix();
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

void Missile::update(float fdt)
{
   if (alive) {
      float speed = 800.0;

      pos = pos + dir * speed * fdt;

      if (dist(start, pos) > 500)
         alive = false;
   }
}

void Missile::draw()
{
   if (alive) {
      glPushMatrix();
      glTranslatef(pos.x, pos.y, 0.0);
      glScalef(spriteZoom, spriteZoom, 1.0);
      glRotatef(toDeg(atan2(dir.y, dir.x)), 0, 0, 1);
      
      sprites16.draw(9,0); // firebolt
      //sprites16.draw(8,0); // arrow

      //sprites16x32.draw(18,1); // pillar

      //sprites32.draw(12,2); // mini trident
      //sprites32.draw(11,0); // small fireball
      //sprites32.draw(6,0); // ball?
      //sprites32.draw(5,0); // bomb (dont rotate)
      //sprites32.draw(7,0); // boomerang (dont rotate, animation 7 - 10)
      //sprites32.draw(12,0); // strange 4 orb thing. rotate and animate 12-13
      //sprites64.draw(7,0); // big ass spear
      glPopMatrix();
   }
}

void Item::init(vec2 pos, Type type)
{
    this->pos = pos;
    this->type = type;
    alive = true;

   switch(type) {
      case Item::GreenRupee :
         anim = &greenRupeeAnim;
         break;
      case Item::RedRupee :
         anim = &redRupeeAnim;
         break;
      case Item::BlueRupee :
         anim = &blueRupeeAnim;
         break;
      case Item::Explosion :
         anim = &explosionAnim;
         break;
      default:
         printf("Error: invalid Item (Type %d) to animate.\n", type);
   }
}

void Item::update(float fdt, Player &player)
{
   if (alive) {
      //if within 40 pixels make not alive
      if (dist(player.pos, pos) < 40)
         alive = false;
   }
}

void Item::draw()
{
   if (alive && anim) {
      glPushMatrix();
      glTranslatef(pos.x, pos.y, 0.0);
      glScalef(spriteZoom, spriteZoom, 1.0);

      anim->draw();
      //sprites16.draw(13,0); // green rupee 1

      glPopMatrix();
   }  
}

void NPC::init(vec2 pos, Type type)
{
   this->type = type;
   this->pos = pos;
   moving = false;
   alive = true;
   dir = vec2(0, 0);
   switch(type) {
      case NPC::Thief :
         this->anim = &thiefAnim;
         break;
      case NPC::Princess :
         this->anim = &princessAnim;
         break;
      case NPC::Fairy :
         this->anim = &fairyAnim;
         break;
      case NPC::Skeleton :
         this->anim = &skeletonAnim;
         break;
      case NPC::Cyclops :
         this->anim = &cyclopsAnim;
         break;
      case NPC::Bat :
         this->anim = &batAnim;
         break;
      case NPC::Bird :
         this->anim = &birdAnim;
         break;
      case NPC::Squirrel :
         this->anim = &squirrelAnim;
         break;
      case NPC::Chicken :
         this->anim = &chickenAnim;
         break;
      case NPC::Vulture :
         this->anim = &vultureAnim;
         break;
      case NPC::Bush :
         this->anim = &bushAnim;
         break;
      case NPC::Cactus :
         this->anim = &cactusAnim;
         break;
      case NPC::BigFairy :
         this->anim = &bigFairyAnim;
         break;
      case NPC::Wizard :
         this->anim = &wizardAnim;
         break;
      case NPC::Ganon :
         this->anim = &ganonAnim;
         break;
      case NPC::Goblin :
         this->anim = &goblinAnim;
         break;
      default:
         printf("Error: You forgot to add the Animation to NPC::init !\n");
   }
}

void NPC::update(float fdt, Player &player)
{
   if(alive) {
      float speed = 300;
      vec2 dist = to(pos, player.pos);
      bool ismoving = dist.length() > 40;
      if(!moving && ismoving)
         this->anim->animStart = SDL_GetTicks();
      moving = ismoving;
      dir = normalize(dist);
      if(moving)
         this->pos = dir*speed*fdt + pos;
   }
}

void NPC::draw()
{
   if(alive && anim) {
      glPushMatrix();
      glTranslatef(pos.x, pos.y, 0.0);
      glScalef(spriteZoom, spriteZoom, 1.0);
      anim->draw(dir, moving);
      glPopMatrix();
   }
}

int Animation::dirFace(vec2 dir)
{
   int adir = 0;
   if (fabs(dir.x) >= fabs(dir.y)) {
      if(dir.x > 0)
         adir = 1; //right
      else
         adir = 3; //left
   } else {
      if(dir.y > 0)
         adir = 0; // up
      else
         adir = 2; //down
   }
   return adir;
}

int Animation::dirFaceLR(vec2 dir)
{
   int adir = 0;
   if(dir.x > 0)
      adir = 1; //right
   else
      adir = 3; //left
   return adir;
}

void Animation::init(Sprite *sprite, Type type, bool alwaysAnim)
{
   this->sprite = sprite;
   this->type = type;
   this->alwaysAnim = alwaysAnim;
}

void Animation::draw(vec2 dir, bool moving)
{
   int dirIndex = 2; //down by default
   switch(this->type) {
      case Animation::Normal :
         dirIndex = dirFace(dir);
         break;
      case Animation::LeftRight :
         dirIndex = dirFaceLR(dir);
         break;
      case Animation::Forward :
         //printf("anim::draw forward todo\n");
         dirIndex = Animation::Down;
         break;
      default:
         printf("Error: anim::draw invalid animation type");
   }
   int frames = dirs[dirIndex].size();
   if(frames > 0) {
      int frame;
      if(alwaysAnim)
         frame = ((SDL_GetTicks() - animStart) / speed) % frames;
      else if(!moving)
         frame = 0;
      else
         frame = 1 + ((SDL_GetTicks() - animStart) / speed) % (frames - 1);
      sprite->draw(dirs[dirIndex][frame].x, 
         dirs[dirIndex][frame].y);
   } else
      printf("Error: unable to draw animation\n");
}

void Animation::draw()
{
   draw(vec2(0.1,-0.9), true);
}