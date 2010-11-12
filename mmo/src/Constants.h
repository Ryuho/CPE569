
#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

namespace constants {
   const float playerSpeed = 400; // Number of pixels per second the player can move
   const float npcAttackSpeed = 300;
   const float npcWalkSpeed = 100;
   const int numArrows = 30; // Number of arrows that are fired in the special attack
   const int arrowCooldown = 150; // Number of ms between arrow shots
   const int specialCooldown = 2000; // ms between special attacks

   const float maxProjectileDist = 500;
   const float projectileSpeed = 800;

   const int playerPredictTicks = 200;

	const float PI = 3.14159265359f;

   const int playerMaxHp = 100;
	const int worldHeight = 5000;
	const int worldWidth = 5000;
   
   const float playerRadius = 20.0f; // used for collision detection
   const float arrowRadius = 16.0f; // used for collision detection
   const float NPCRadius = 20.0f; // used for collision detection
   const float attackRange = 50.0f;
   const float maxNpcMoveDist = 5000.0f;
   const float npcAggroRange = 400.0f;


   namespace ObjectType { enum {
      Player=0, NPC, Missile, Item,
   };}
   namespace NPCType { enum {
      Thief=0, Princess, Fairy, Skeleton, Cyclops,
      Bat, Bird, Squirrel, Chicken, Vulture, Bush, Cactus,
      BigFairy, Wizard, Ganon, Goblin,
   };}
   namespace ItemType { enum {
      GreenRupee=0, RedRupee, BlueRupee, Explosion, Stump, Heart
   };}
   namespace MissileType { enum {
      Arrow=0,
   };}
   namespace AIType { enum {
      Stopped=0, Walking, Attacking
   };}

}

#endif
