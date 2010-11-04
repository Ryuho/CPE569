#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

namespace constants {
   const float playerSpeed = 400; // Number of pixels per second the player can move
   const float npcSpeed = 300;
   const int numArrows = 30; // Number of arrows that are fired in the special attack
   const int arrowCooldown = 150; // Number of ms between arrow shots
   const int specialCooldown = 2000; // ms between special attacks

   const float maxProjectileDist = 500;
   const float projectileSpeed = 800;

   const int playerPredictTicks = 200;

	const float PI = 3.14159265359f;

   namespace ObjectType { enum {
      Player, NPC, Missile, Item,
   };}
   namespace NPCType { enum {
      Thief, Princess, Fairy, Skeleton, Cyclops,
      Bat, Bird, Squirrel, Chicken, Vulture, Bush, Cactus,
      BigFairy, Wizard, Ganon, Goblin,
   };}
   namespace ItemType { enum {
      GreenRupee, RedRupee, BlueRupee, Explosion
   };}
   namespace MissileType { enum {
      Arrow
   };}

}

#endif
