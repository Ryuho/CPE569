
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
   const float areaOfInfluenceRadius = 800;

   const int predictTicks = 200;
   const int deltaRegion = 1;

   const int regionSize = 1000;
   const int noDrawTicks = 400;
   const int npcOutOfCombatHpPerTick = 1;
   const int playerHpPerTick = 1;

   #define PI 3.14159265359

   const int npcMaxHp = 120;
   const int playerMaxHp = 200;
	const int worldHeight = 5000;
	const int worldWidth = 5000;
   const int regionXSize = ((int)(worldWidth / regionSize))*regionSize == worldWidth ?
      ((int)(worldWidth / regionSize)) : ((int)(worldWidth / regionSize)) + 1;
   const unsigned regionYSize = ((int)(worldHeight / regionSize))*regionSize == worldHeight ?
      ((int)(worldHeight / regionSize)) : ((int)(worldHeight / regionSize)) + 1;
   const int totalRegions = regionXSize * regionYSize;

   const int heartValue = playerMaxHp/4;
   
   const float playerRadius = 20.0f; // used for collision detection
   const float arrowRadius = 16.0f; // used for collision detection
   const float NPCRadius = 20.0f; // used for collision detection
   const float attackRange = 200.0f;
   const float maxNpcMoveDist = 2000.0f;
   const float npcAggroRange = 355.0f;

   const int greenRupeeValue = 1;
   const int blueRupeeValue = 3;
   const int redRupeeValue = 5;

   namespace AnimType { enum { 
      LeftRight=0, Forward=1, Normal=2
   };}
   namespace Direction { enum {
      Up=0, Right=1, Down=2, Left=3
   };}

   namespace ObjectType { enum {
      Player=0, NPC=1, Missile=2, Item=3, 
      ObjectTypeCount=4 //make sure it's last
   };}
   namespace NPCType { enum {
      Chicken=0, Squirrel=1, Bird=2, Bush=3, Fairy=4, Cactus=5, Princess=6,
      Vulture=7, Bat=8,  Thief=9, Goblin=10, Skeleton=11,  Cyclops=12,
      Wizard=13, BigFairy=14, Ganon=15, 
   };}
   namespace ItemType { enum {
      GreenRupee=0, RedRupee=1, BlueRupee=2, Explosion=3, Stump=4, 
      Heart=5, Teleportor=6
   };}
   namespace MissileType { enum {
      Arrow=0,
   };}
   namespace AIType { enum {
      Stopped=0, Walking=1, Attacking=2
   };}

   namespace PacketType { enum {
      // Make new packet types explicit
      position = 1,
      message = 2,
      connect = 3,
      signal = 4,
	   arrow = 5,
      initialize = 6,
      healthChange = 7,
      click = 8,
      serialPlayer = 9,
      serialItem = 10,
      serialNPC = 11,
      serialMissile = 12,
      serverList = 13,
      changePvp = 14,
   };}
}

#endif
