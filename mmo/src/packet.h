#ifndef _PACKET_H_
#define _PACKET_H_

#include <boost/shared_ptr.hpp>
#include "socket.h"
#include "matrix.h"
#include <string>

namespace pack {
using namespace mat;

enum PacketType {
   position = 1,
   message = 2,
   connect = 3,
   signal = 4,
   // Add new packet types here, make values explicit
	arrow = 5,
   initialize = 6,
   healthChange = 7,
   click = 8,
};

// Simple structure for reading/writing using sockets.
struct Packet {
   Packet(int size, int type) : size(size), type(type) {}
   sock::Packet data;
   int size, type;
   
   bool sendTo(sock::Connection conn) {
      sock::Packet header;
      header.writeInt(size).writeInt(type);
      return conn.send(header) && conn.send(data);
   }
};

// Read a packet from a connection
inline Packet readPacket(sock::Connection conn) {
   int size, type;
   sock::Packet header;
   
   if (conn.recv(header, 8)) {
      header.readInt(size).readInt(type);
      Packet ret(size, type);
      if (conn.recv(ret.data, ret.size))
         return ret;
   }
   
   return Packet(0,0);
}

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
      Packet p(24, position);
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
      Packet p(12, click);
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
      Packet p(0, message);
      p.data.writeStdStr(str);
      p.size = p.data.size();
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
      Packet p(12, connect);
      p.data.writeInt(id).writeInt(worldHeight).writeInt(worldWidth);
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
      Packet p(8, signal);
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
      Packet p(20, arrow);
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
      Packet p(32, initialize);
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
      Packet p(8, healthChange);
      p.data.writeInt(id).writeInt(hp);
      return p;
   }
};


} // end pack namespace

#endif
