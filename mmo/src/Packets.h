#ifndef __PACKETS_H__
#define __PACKETS_H__

#include "packet.h"
#include "Constants.h"

namespace pack {
   using namespace constants;
// Actual packets
// You can even declare these somewhere else, just put a value in the enum

// Update of a player position. 
// id is not used when sending to server (can't trust pesky client)
struct Position {
   vec2 pos, dir;
   int id, moving;
   Position() {}
   Position(vec2 pos, vec2 dir, bool moving, int id) 
      : pos(pos), dir(dir), moving(moving), id(id) {}
   Position(Packet &p) {
      p.data.readInt(id).readInt(moving)
         .readFloat(pos.x).readFloat(pos.y)
         .readFloat(dir.x).readFloat(dir.y)
         .reset();
   }
   Packet makePacket() {
      Packet p(PacketType::position);
      p.data.writeInt(id).writeInt(moving)
         .writeFloat(pos.x).writeFloat(pos.y)
         .writeFloat(dir.x).writeFloat(dir.y);
      return p;
   }
};

struct Click {
   Click() : id(0) {}
   Click(vec2 pos, int id) 
      : pos(pos), id(id) {}
   Click(Packet &p) {
      p.data.readInt(id).readFloat(pos.x)
         .readFloat(pos.y).reset();
   }
   Packet makePacket() {
      Packet p(PacketType::click);
      p.data.writeInt(id).writeFloat(pos.x)
         .writeFloat(pos.y);
      return p;
   }

   vec2 pos;
   int id;
};

// Player/Server sends a text message?
struct Message {
   std::string str;
   Message() {}
   Message(std::string str) : str(str) {}
   Message(Packet &p) {
      p.data.readStdStr(str).reset();
   }
   Packet makePacket() {
      Packet p(PacketType::message);
      p.data.writeStdStr(str);
      return p;
   }
};

// Sent to the connecting client to tell them their id.
struct Connect {
   int id;
	int worldHeight;
	int worldWidth;
   Connect() : id(0) {}
   Connect(int id) : id(id) {}
	Connect(int id, int height, int width) : id(id), worldHeight(height), worldWidth(width) {}
   Connect(Packet &p) {
      p.data.readInt(id).readInt(worldHeight).readInt(worldWidth).reset();
   }
   Packet makePacket() {
      Packet p(PacketType::connect);
      p.data.writeInt(id).writeInt(worldHeight).writeInt(worldWidth);
      return p;
   }
};

struct Pvp
{
   int id; //player id
   int isPvpMode;
   Pvp() : id(0), isPvpMode(false) {}
   Pvp(int id, bool isPvpMode) : id(id), isPvpMode(isPvpMode) {}
   Pvp(Packet &p) {
      p.data.readInt(id).readInt(isPvpMode).reset();
   }
   Packet makePacket() {
      Packet p(PacketType::changePvp);
      p.data.writeInt(id).writeInt(isPvpMode);
      return p;
   }
};

// Used to combine all simple signals into one packet
struct Signal {
   enum { 
      hello = 1,        // Not used yet... A simple ping
      remove = 2,      // Indicates the object with id is no longer seen
		special = 3,		// The player id(val) casts a special attack
      hurtme = 4,       // damages the player
      changeExp = 5,
      changeRupee = 6,
   };
   int sig, val;
   Signal() : sig(0) {}
   Signal(int sig) : sig(sig) {}
   Signal(int sig, int val) : sig(sig), val(val) {}
   Signal(Packet &p) {
      p.data.readInt(sig).readInt(val).reset();
   }
   Packet makePacket() {
      Packet p(PacketType::signal);
      p.data.writeInt(sig).writeInt(val);
      return p;
   }
};

// used for an arrow
struct Arrow {
	vec2 orig;
	vec2 direction;
   int id;
   Arrow() {}
   Arrow(vec2 dir) : orig(0), direction(dir), id(0) {}
   Arrow(vec2 dir, int id) : orig(0), direction(dir), id(id) {}
	Arrow(vec2 dir, vec2 orig, int id) : orig(orig), direction(dir), id(id) {}
   Arrow(Packet &p) {
      p.data.readInt(id).readFloat(orig.x).readFloat(orig.y).readFloat(direction.x).readFloat(direction.y).reset();
   }
   Packet makePacket() {
      Packet p(PacketType::arrow);
      p.data.writeInt(id).writeFloat(orig.x).writeFloat(orig.y).writeFloat(direction.x).writeFloat(direction.y);
      return p;
   }
};

// A general packet for starting a player with all existing objects in the scene.
struct Initialize {
   int id, type, subType, hp;//, worldWidth, worldHeight;
   vec2 pos, dir;
   Initialize() {}
	Initialize(int id, int type, int subType, vec2 pos, vec2 dir, int hp)
      : id(id), type(type), subType(subType), pos(pos), dir(dir), hp(hp) {}
   Initialize(Packet &p) {
      p.data.readInt(id).readInt(type).readInt(subType).readFloat(pos.x).readFloat(pos.y).readFloat(dir.x).readFloat(dir.y).readInt(hp).reset();
   }
   Packet makePacket() {
      Packet p(PacketType::initialize);
      p.data.writeInt(id).writeInt(type).writeInt(subType).writeFloat(pos.x).writeFloat(pos.y).writeFloat(dir.x).writeFloat(dir.y).writeInt(hp);
      return p;
   }
};

struct HealthChange {
   int id, hp;
   HealthChange(int id, int hp) : id(id), hp(hp) {}
   HealthChange(Packet &p) {
      p.data.readInt(id).readInt(hp).reset();
   }
   Packet makePacket() {
      Packet p(PacketType::healthChange);
      p.data.writeInt(id).writeInt(hp);
      return p;
   }
};

//server to server packet
struct ServerList {
   std::vector<unsigned long> uLongList;
   ServerList(std::vector<unsigned long> uList) : uLongList(uList) {}
   ServerList(Packet &p) {
      int size;
      p.data.readInt(size);
      for(int i = 0; i < size; i++){
         unsigned long temp;
         p.data.readLong(temp);
         uLongList.push_back(temp);
      }
      p.data.reset();
   }
   Packet makePacket() {
      Packet p(PacketType::serverList);
      p.data.writeInt(uLongList.size());
      for(unsigned i = 0; i < uLongList.size(); i++){
         p.data.writeLong(uLongList.at(i));
      }
      return p;
   }
};

} //end namespace

#endif // __PACKETS_H__