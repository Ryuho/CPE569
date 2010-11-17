
#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

namespace constants {
   const float playerSpeed = 400; // Number of pixels per second the player can move
   const float npcAttackSpeed = 300;
   const float npcWalkSpeed = 130;
   const int numArrows = 30; // Number of arrows that are fired in the special attack
   const int arrowCooldown = 150; // Number of ms between arrow shots
   const int specialCooldown = 2000; // ms between special attacks

   const float maxProjectileTicks = 500;
   const float projectileSpeed = 800;

   const int predictTicks = 200;

   const int regionSize = 500;
   const int noDrawTicks = 1000;

   #define PI 3.14159265359

   const int npcMaxHp = 120;
   const int playerMaxHp = 200;
	const int worldHeight = 5000;
	const int worldWidth = 5000;
   
   const float playerRadius = 20.0f; // used for collision detection
   const float arrowRadius = 16.0f; // used for collision detection
   const float NPCRadius = 20.0f; // used for collision detection
   const float attackRange = 200.0f;
   const float maxNpcMoveDist = 2000.0f;
   const float npcAggroRange = 325.0f;

   const int greenRupeeValue = 1;
   const int blueRupeeValue = 3;
   const int redRupeeValue = 5;

   namespace AnimType { enum { 
      LeftRight=0, Forward, Normal 
   };}
   namespace Direction { enum {
      Up=0, Right, Down, Left 
   };}

   namespace ObjectType { enum {
      Player=0, NPC, Missile, Item,
   };}
   namespace NPCType { enum {
      Thief=0, Princess, Fairy, Skeleton, Cyclops,
      Bat, Bird, Squirrel, Chicken, Vulture, Bush, Cactus,
      BigFairy, Wizard, Ganon, Goblin,
   };}
   namespace ItemType { enum {
      GreenRupee=0, RedRupee, BlueRupee, Explosion, Stump, Heart, Teleportor
   };}
   namespace MissileType { enum {
      Arrow=0,
   };}
   namespace AIType { enum {
      Stopped=0, Walking, Attacking
   };}

}

#endif
