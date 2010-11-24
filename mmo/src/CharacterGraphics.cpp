#include "Characters.h"
#include "GLUtil.h"
#include "Sprite.h"
#include "World.h"
#include "Constants.h"

using namespace constants;
using namespace std;

namespace {
   Texture spriteTex;
   Sprite sprites32;
   Sprite sprites64;
   Sprite sprites16x32;
   Sprite sprites16;
   Sprite sprites8;

   int spriteZoom = 3;

   bool initalized = false;

   //NPCs (animated)
   Animation linkAnim;
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

   //Items (animated)
   Animation greenRupeeAnim;
   Animation redRupeeAnim;
   Animation blueRupeeAnim;
   Animation explosionAnim;
   //Items (non-animated)
   Animation stumpAnim;
   Animation heartAnim;
   Animation heartSolidAnim;
   Animation teleportorAnim;

   /*
   //TODO (animated)
   Animation teleporterAnim;
   Animation boomerangSpinAnim;
   Animation waterBallsAnim;
   Animation torchAnim;

   //TODO (non-animated)
   //ordered by size descending
   Animation bottledFairyAnim;
   Animation bottleAnim;
   Animation magicBallAnim;
   Animation blueAmuletAnim;
   Animation greenAmuletAnim;
   Animation blueFloorAnim;
   Animation floorButtonAnim;
   Animation boomerangAnim;
   Animation keyAnim;
   Animation bombAnim;
   Animation forkAnim;
   Animation treasureAnim;
   Animation openTreasureAnim;
   Animation greenPlantAnim;
   Animation cutPlantAnim;
   Animation skullAnim;
   Animation skeletonHeadAnim;
   Animation potAnim;
   Animation appleAnim;
   Animation keyAnim;

   Animation greyStatueAnim;
   Animation brownPillarAnim;
   Animation eyeStatueAnim;
   Animation swordInStoneAnim;
   Animation linkDeadAnim;
   Animation princessDeadAnim;
   Animation troughAnim;

   Animation fairyFountainAnim;
   Animation spikeBlockAnim;
   Animation rockAnim;
   Animation gravestoneAnim;
   Animation urnAnim;
   Animation pillarAnim;

   Animation fireBallsAnim;
   Animation spearAnim;

   Animation treeAnim;
   Animation treeHouseAnim;
   */
}

void drawQuad(vec2 bl, vec2 tr)
{
   glBegin(GL_QUADS);

   glVertex2f(bl.x, bl.y);
   glVertex2f(tr.x, bl.y);
   glVertex2f(tr.x, tr.y);
   glVertex2f(bl.x, tr.y);

   glEnd();
}

void drawHpBar(float percent, int width, int height, bool isPvp)
{
   float center = (width*2 * percent) - width;

   if (percent == 1.0)
      return;

   glDisable(GL_TEXTURE_2D);
   if(isPvp)
      glColor4ub(255, 200, 0, 100); //orangeish yellow
   else
      glColor4ub(0, 255, 0, 100); //green
   drawQuad(vec2(-width, height), vec2(center, height+2));
   glColor4ub(255, 0, 0, 100);
   drawQuad(vec2(center, height), vec2(width, height+2));
   glColor4ub(255, 255, 255, 255);
   glEnable(GL_TEXTURE_2D);
}

void Player::draw()
{
   int frame, adir;
   
   if (!alive)
      return;

   if (moving)
      frame = ((getTicks() - animStart) / 100) % 8;
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

   glTranslatef(-1,0,0); // needs this offset or else it looks strange
   drawHpBar((float)hp / playerMaxHp, 11, 13, pvp);

   glPopMatrix();

}

void Missile::draw()
{
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

void Item::initGraphics()
{
   switch(type) {
      case ItemType::GreenRupee :
         anim = &greenRupeeAnim;
         break;
      case ItemType::RedRupee :
         anim = &redRupeeAnim;
         break;
      case ItemType::BlueRupee :
         anim = &blueRupeeAnim;
         break;
      case ItemType::Explosion :
         anim = &explosionAnim;
         break;
      case ItemType::Stump :
         anim = &stumpAnim;
         break;
      case ItemType::Heart :
         anim = &heartAnim;
         break;
	  case ItemType::Teleportor :
		  anim = &teleportorAnim;
		  break;
	  default:
         printf("Error: invalid Item (Type %d) to animate.\n", type);
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

void NPC::initGraphics()
{
   this->anim = type == NPCType::Thief     ? &thiefAnim :
                type == NPCType::Princess  ? &princessAnim :
                type == NPCType::Fairy     ? &fairyAnim :
                type == NPCType::Skeleton  ? &skeletonAnim :
                type == NPCType::Cyclops   ? &cyclopsAnim :
                type == NPCType::Bat       ? &batAnim :
                type == NPCType::Bird      ? &birdAnim :
                type == NPCType::Squirrel  ? &squirrelAnim :
                type == NPCType::Chicken   ? &chickenAnim :
                type == NPCType::Vulture   ? &vultureAnim :
                type == NPCType::Cactus    ? &cactusAnim :
                type == NPCType::BigFairy  ? &bigFairyAnim :
                type == NPCType::Wizard    ? &wizardAnim :
                type == NPCType::Ganon     ? &ganonAnim :
                type == NPCType::Goblin    ? &goblinAnim :
                type == NPCType::Bush      ? &bushAnim :
                (printf("Unknown NPC type: %d\n", type), (Animation*)0);
}

void NPC::resetAnimation()
{
   this->anim->animStart = getTicks();
}

void NPC::draw()
{
   if(alive && anim) {
      glPushMatrix();
      glTranslatef(pos.x, pos.y, 0.0);
      glScalef(spriteZoom, spriteZoom, 1.0);
      anim->draw(dir, moving);
      glTranslatef(-1,0,0);
      drawHpBar((float)hp / npcMaxHp, 11, 13, true);
      glPopMatrix();
   }
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

      linkAnim.init(&sprites32, AnimType::Normal, false);
      linkAnim.speed = 150;
      linkAnim.dirs[Direction::Up].push_back(vec2i(1,0));
      linkAnim.dirs[Direction::Up].push_back(vec2i(0,2));
      linkAnim.dirs[Direction::Up].push_back(vec2i(1,2));
      linkAnim.dirs[Direction::Up].push_back(vec2i(2,2));
      linkAnim.dirs[Direction::Up].push_back(vec2i(3,2));
      linkAnim.dirs[Direction::Up].push_back(vec2i(4,2));
      linkAnim.dirs[Direction::Up].push_back(vec2i(5,2));
      linkAnim.dirs[Direction::Up].push_back(vec2i(6,2));
      linkAnim.dirs[Direction::Up].push_back(vec2i(7,2));
      linkAnim.dirs[Direction::Right].push_back(vec2i(2,0));
      linkAnim.dirs[Direction::Right].push_back(vec2i(1,3));
      linkAnim.dirs[Direction::Right].push_back(vec2i(2,3));
      linkAnim.dirs[Direction::Right].push_back(vec2i(3,3));
      linkAnim.dirs[Direction::Right].push_back(vec2i(4,3));
      linkAnim.dirs[Direction::Right].push_back(vec2i(5,3));
      linkAnim.dirs[Direction::Right].push_back(vec2i(6,3));
      linkAnim.dirs[Direction::Right].push_back(vec2i(7,3));
      linkAnim.dirs[Direction::Right].push_back(vec2i(0,3));
      linkAnim.dirs[Direction::Down].push_back(vec2i(4,0));
      linkAnim.dirs[Direction::Down].push_back(vec2i(0,4));
      linkAnim.dirs[Direction::Down].push_back(vec2i(1,4));
      linkAnim.dirs[Direction::Down].push_back(vec2i(2,4));
      linkAnim.dirs[Direction::Down].push_back(vec2i(3,4));
      linkAnim.dirs[Direction::Down].push_back(vec2i(4,4));
      linkAnim.dirs[Direction::Down].push_back(vec2i(5,4));
      linkAnim.dirs[Direction::Down].push_back(vec2i(6,4));
      linkAnim.dirs[Direction::Down].push_back(vec2i(7,4));
      linkAnim.dirs[Direction::Left].push_back(vec2i(0,0));
      linkAnim.dirs[Direction::Left].push_back(vec2i(1,1));
      linkAnim.dirs[Direction::Left].push_back(vec2i(2,1));
      linkAnim.dirs[Direction::Left].push_back(vec2i(3,1));
      linkAnim.dirs[Direction::Left].push_back(vec2i(4,1));
      linkAnim.dirs[Direction::Left].push_back(vec2i(5,1));
      linkAnim.dirs[Direction::Left].push_back(vec2i(6,1));
      linkAnim.dirs[Direction::Left].push_back(vec2i(7,1));
      linkAnim.dirs[Direction::Left].push_back(vec2i(0,1));

      //////////////////////////////////////////////
      /////////////// NPC Animations ///////////////
      //////////////////////////////////////////////
      thiefAnim.init(&sprites16x32, AnimType::Normal, false);
      thiefAnim.speed = 175;
      thiefAnim.dirs[Direction::Up].push_back(vec2i(11, 6)); //up
      thiefAnim.dirs[Direction::Up].push_back(vec2i(0,  6));
      thiefAnim.dirs[Direction::Up].push_back(vec2i(1,  6));
      thiefAnim.dirs[Direction::Right].push_back(vec2i(6,  6)); //right
      thiefAnim.dirs[Direction::Right].push_back(vec2i(7,  6));
      thiefAnim.dirs[Direction::Right].push_back(vec2i(6,  6));
      thiefAnim.dirs[Direction::Down].push_back(vec2i(10, 6)); //down
      thiefAnim.dirs[Direction::Down].push_back(vec2i(4,  6));
      thiefAnim.dirs[Direction::Down].push_back(vec2i(5,  6));
      thiefAnim.dirs[Direction::Left].push_back(vec2i(3,  6)); //left
      thiefAnim.dirs[Direction::Left].push_back(vec2i(2,  6));
      thiefAnim.dirs[Direction::Left].push_back(vec2i(3,  6));

      princessAnim.init(&sprites16x32, AnimType::Normal, false);
      princessAnim.speed = 200;
      princessAnim.dirs[Direction::Up].push_back(vec2i(18, 6)); //up
      princessAnim.dirs[Direction::Up].push_back(vec2i(19, 6));
      princessAnim.dirs[Direction::Up].push_back(vec2i(18, 6));
      princessAnim.dirs[Direction::Right].push_back(vec2i(16, 6)); //right
      princessAnim.dirs[Direction::Right].push_back(vec2i(17, 6));
      princessAnim.dirs[Direction::Right].push_back(vec2i(16, 6));
      princessAnim.dirs[Direction::Down].push_back(vec2i(14, 6)); //down
      princessAnim.dirs[Direction::Down].push_back(vec2i(15, 6));
      princessAnim.dirs[Direction::Down].push_back(vec2i(14, 6)); 
      princessAnim.dirs[Direction::Left].push_back(vec2i(20, 6)); //left
      princessAnim.dirs[Direction::Left].push_back(vec2i(21, 6));
      princessAnim.dirs[Direction::Left].push_back(vec2i(20, 6));

      fairyAnim.init(&sprites16, AnimType::LeftRight, true);
      fairyAnim.dirs[Direction::Right].push_back(vec2i(22, 0)); //right
      fairyAnim.dirs[Direction::Right].push_back(vec2i(23, 0));
      fairyAnim.dirs[Direction::Left].push_back(vec2i(22, 1)); //left
      fairyAnim.dirs[Direction::Left].push_back(vec2i(23, 1));

      skeletonAnim.init(&sprites16x32, AnimType::Normal, false);
      skeletonAnim.speed = 200;
      skeletonAnim.dirs[Direction::Up].push_back(vec2i(30, 0)); //up
      skeletonAnim.dirs[Direction::Up].push_back(vec2i(31, 0));
      skeletonAnim.dirs[Direction::Up].push_back(vec2i(30, 0));
      skeletonAnim.dirs[Direction::Right].push_back(vec2i(24, 0)); //right
      skeletonAnim.dirs[Direction::Right].push_back(vec2i(25, 0));
      skeletonAnim.dirs[Direction::Right].push_back(vec2i(24, 0));
      skeletonAnim.dirs[Direction::Down].push_back(vec2i(28, 0)); //down
      skeletonAnim.dirs[Direction::Down].push_back(vec2i(29, 0));
      skeletonAnim.dirs[Direction::Down].push_back(vec2i(28, 0)); 
      skeletonAnim.dirs[Direction::Left].push_back(vec2i(27, 0)); //left
      skeletonAnim.dirs[Direction::Left].push_back(vec2i(26, 0));
      skeletonAnim.dirs[Direction::Left].push_back(vec2i(27, 0));

      cyclopsAnim.init(&sprites32, AnimType::Normal, false);
      cyclopsAnim.speed = 200;
      cyclopsAnim.dirs[Direction::Up].push_back(vec2i(12, 7)); //up
      cyclopsAnim.dirs[Direction::Up].push_back(vec2i(11, 7));
      cyclopsAnim.dirs[Direction::Up].push_back(vec2i(12, 7));
      cyclopsAnim.dirs[Direction::Up].push_back(vec2i(13, 7));
      cyclopsAnim.dirs[Direction::Up].push_back(vec2i(12, 7));
      cyclopsAnim.dirs[Direction::Right].push_back(vec2i(7,  7));  //right
      cyclopsAnim.dirs[Direction::Right].push_back(vec2i(6,  7));
      cyclopsAnim.dirs[Direction::Right].push_back(vec2i(7,  7));
      cyclopsAnim.dirs[Direction::Down].push_back(vec2i(4,  7)); //down
      cyclopsAnim.dirs[Direction::Down].push_back(vec2i(8,  7));
      cyclopsAnim.dirs[Direction::Down].push_back(vec2i(9,  7)); 
      cyclopsAnim.dirs[Direction::Down].push_back(vec2i(10, 7));
      cyclopsAnim.dirs[Direction::Down].push_back(vec2i(9,  7)); 
      cyclopsAnim.dirs[Direction::Left].push_back(vec2i(12, 8)); //left
      cyclopsAnim.dirs[Direction::Left].push_back(vec2i(13, 8));
      cyclopsAnim.dirs[Direction::Left].push_back(vec2i(12, 8));

      batAnim.init(&sprites16, AnimType::Forward, true);
      batAnim.speed = 175;
      batAnim.dirs[Direction::Down].push_back(vec2i(24, 18)); //down
      batAnim.dirs[Direction::Down].push_back(vec2i(24, 19));

      birdAnim.init(&sprites16, AnimType::LeftRight, true);
      birdAnim.speed = 150;
      birdAnim.dirs[Direction::Right].push_back(vec2i(0, 10)); //right
      birdAnim.dirs[Direction::Right].push_back(vec2i(1, 10));
      birdAnim.dirs[Direction::Left].push_back(vec2i(0, 11)); //left
      birdAnim.dirs[Direction::Left].push_back(vec2i(1, 11));

      squirrelAnim.init(&sprites16x32, AnimType::LeftRight, false);
      squirrelAnim.speed = 150;
      squirrelAnim.dirs[Direction::Right].push_back(vec2i(2, 5)); //right
      squirrelAnim.dirs[Direction::Right].push_back(vec2i(3, 5));
      squirrelAnim.dirs[Direction::Right].push_back(vec2i(2, 5));
      squirrelAnim.dirs[Direction::Left].push_back(vec2i(4, 5)); //left
      squirrelAnim.dirs[Direction::Left].push_back(vec2i(5, 5));
      squirrelAnim.dirs[Direction::Left].push_back(vec2i(4, 5));

      chickenAnim.init(&sprites32, AnimType::LeftRight, false);
      chickenAnim.speed = 175;
      chickenAnim.dirs[Direction::Right].push_back(vec2i(3, 5)); //right
      chickenAnim.dirs[Direction::Right].push_back(vec2i(4, 5));
      chickenAnim.dirs[Direction::Right].push_back(vec2i(5, 5));
      chickenAnim.dirs[Direction::Right].push_back(vec2i(3, 5));
      chickenAnim.dirs[Direction::Left].push_back(vec2i(6, 5)); //left
      chickenAnim.dirs[Direction::Left].push_back(vec2i(7, 5));
      chickenAnim.dirs[Direction::Left].push_back(vec2i(8, 5));
      chickenAnim.dirs[Direction::Left].push_back(vec2i(6, 5));

      vultureAnim.init(&sprites32, AnimType::Forward, true);
      vultureAnim.dirs[Direction::Down].push_back(vec2i(9, 5)); //down
      vultureAnim.dirs[Direction::Down].push_back(vec2i(10, 5));
      vultureAnim.dirs[Direction::Down].push_back(vec2i(11, 5));
      vultureAnim.dirs[Direction::Down].push_back(vec2i(10, 5));

      bushAnim.init(&sprites32, AnimType::Forward, false);
      bushAnim.speed = 300;
      bushAnim.dirs[Direction::Down].push_back(vec2i(12, 3)); //down
      bushAnim.dirs[Direction::Down].push_back(vec2i(11, 3));
      bushAnim.dirs[Direction::Down].push_back(vec2i(12, 3));
      bushAnim.dirs[Direction::Down].push_back(vec2i(13, 3));
      bushAnim.dirs[Direction::Down].push_back(vec2i(12, 3));

      cactusAnim.init(&sprites16x32, AnimType::Forward, false);
      cactusAnim.speed = 300;
      cactusAnim.dirs[Direction::Down].push_back(vec2i(20, 0)); //down
      cactusAnim.dirs[Direction::Down].push_back(vec2i(19, 0));
      cactusAnim.dirs[Direction::Down].push_back(vec2i(20, 0));
      cactusAnim.dirs[Direction::Down].push_back(vec2i(21, 0));
      cactusAnim.dirs[Direction::Down].push_back(vec2i(20, 0));

      bigFairyAnim.init(&sprites32, AnimType::Forward, true);
      bigFairyAnim.speed = 200;
      bigFairyAnim.dirs[Direction::Down].push_back(vec2i(11, 4)); //down
      bigFairyAnim.dirs[Direction::Down].push_back(vec2i(12, 4));
      bigFairyAnim.dirs[Direction::Down].push_back(vec2i(13, 4));
      bigFairyAnim.dirs[Direction::Down].push_back(vec2i(12, 4));

      wizardAnim.init(&sprites16x32, AnimType::Forward, false);
      wizardAnim.speed = 400;
      wizardAnim.dirs[Direction::Down].push_back(vec2i(12, 6)); //down
      wizardAnim.dirs[Direction::Down].push_back(vec2i(13, 6));
      wizardAnim.dirs[Direction::Down].push_back(vec2i(12, 6));

      ganonAnim.init(&sprites32, AnimType::Forward, true);
      ganonAnim.speed = 350;
      ganonAnim.dirs[Direction::Down].push_back(vec2i(9, 4)); //down
      ganonAnim.dirs[Direction::Down].push_back(vec2i(10, 4));

      goblinAnim.init(&sprites32, AnimType::LeftRight, false);
      goblinAnim.speed = 250;
      goblinAnim.dirs[Direction::Right].push_back(vec2i(2, 7)); //right
      goblinAnim.dirs[Direction::Right].push_back(vec2i(3, 7));
      goblinAnim.dirs[Direction::Right].push_back(vec2i(2, 7));
      goblinAnim.dirs[Direction::Left].push_back(vec2i(1, 7)); //left
      goblinAnim.dirs[Direction::Left].push_back(vec2i(0, 7));
      goblinAnim.dirs[Direction::Left].push_back(vec2i(1, 7));

      ///////////////////////////////////////////////
      /////////////// Item Animations ///////////////
      ///////////////////////////////////////////////
      greenRupeeAnim.init(&sprites16, AnimType::Forward, true);
      greenRupeeAnim.speed = 400;
      greenRupeeAnim.dirs[Direction::Down].push_back(vec2i(13,0));
      greenRupeeAnim.dirs[Direction::Down].push_back(vec2i(13,0));
      greenRupeeAnim.dirs[Direction::Down].push_back(vec2i(13,0));
      greenRupeeAnim.dirs[Direction::Down].push_back(vec2i(13,0));
      greenRupeeAnim.dirs[Direction::Down].push_back(vec2i(14,0));
      greenRupeeAnim.dirs[Direction::Down].push_back(vec2i(15,0));

      redRupeeAnim.init(&sprites16, AnimType::Forward, true);
      redRupeeAnim.speed = 400;
      redRupeeAnim.dirs[Direction::Down].push_back(vec2i(16,0));
      redRupeeAnim.dirs[Direction::Down].push_back(vec2i(16,0));
      redRupeeAnim.dirs[Direction::Down].push_back(vec2i(16,0));
      redRupeeAnim.dirs[Direction::Down].push_back(vec2i(16,0));
      redRupeeAnim.dirs[Direction::Down].push_back(vec2i(17,0));
      redRupeeAnim.dirs[Direction::Down].push_back(vec2i(18,0));

      blueRupeeAnim.init(&sprites16, AnimType::Forward, true);
      blueRupeeAnim.speed = 400;
      blueRupeeAnim.dirs[Direction::Down].push_back(vec2i(16,1));
      blueRupeeAnim.dirs[Direction::Down].push_back(vec2i(16,1));
      blueRupeeAnim.dirs[Direction::Down].push_back(vec2i(16,1));
      blueRupeeAnim.dirs[Direction::Down].push_back(vec2i(16,1));
      blueRupeeAnim.dirs[Direction::Down].push_back(vec2i(17,1));
      blueRupeeAnim.dirs[Direction::Down].push_back(vec2i(18,1));

      explosionAnim.init(&sprites64, AnimType::Forward, true);
      explosionAnim.speed = 150;
      explosionAnim.dirs[Direction::Down].push_back(vec2i(7,2));
      explosionAnim.dirs[Direction::Down].push_back(vec2i(7,3));
      explosionAnim.dirs[Direction::Down].push_back(vec2i(7,4));
      explosionAnim.dirs[Direction::Down].push_back(vec2i(7,5));
      explosionAnim.dirs[Direction::Down].push_back(vec2i(7,6));
      explosionAnim.dirs[Direction::Down].push_back(vec2i(7,7));

      stumpAnim.init(&sprites32, AnimType::Forward, true);
      stumpAnim.speed = 1000;
      stumpAnim.dirs[Direction::Down].push_back(vec2i(9,2));

      heartAnim.init(&sprites16, AnimType::Forward, true);
      heartAnim.speed = 1000;
      heartAnim.dirs[Direction::Down].push_back(vec2i(24,4));

	  teleportorAnim.init(&sprites16, AnimType::Forward, true);
     teleportorAnim.speed = 100;
     teleportorAnim.dirs[Direction::Down].push_back(vec2i(24,2));
     //teleportorAnim.dirs[Direction::Down].push_back(vec2i(25,3));
     //teleportorAnim.dirs[Direction::Down].push_back(vec2i(25,2));
     //teleportorAnim.dirs[Direction::Down].push_back(vec2i(24,3));

      initalized = true;
   }
}
