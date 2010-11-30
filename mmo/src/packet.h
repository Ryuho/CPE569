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
   serialPlayer = 9,
   serialItem = 10,
   serialNPC = 11,
   serialMissile = 12,
   serverList = 13,
   changePvp = 14,
};

// Simple structure for reading/writing using sockets.
struct Packet {
   Packet(int type) : type(type) {}
   sock::Packet data;
   int type;
   
   bool sendTo(sock::Connection conn) {
      sock::Packet header;
      header.writeInt(data.size()).writeInt(type);
      return conn.send(header) && conn.send(data);
   }
};

// Read a packet from a connection
inline Packet readPacket(sock::Connection conn) {
   static int lastType, lastSize;
   int size, type;
   sock::Packet header;
   
   if (conn.recv(header, 8)) {
      header.readInt(size).readInt(type);
      Packet ret(type);
      if (conn.tookMultipleReads() || size <= 0 || size > 100 || type <= 0 || type >= 30) {
         printf("*** Header Multiple reads: %d\n", conn.tookMultipleReads());
         printf("*** Strange packet, type: %d, size: %d\n", type, size);
         printf("*** Last packet was type: %d, size: %d\n", lastType, lastSize);
         //return Packet(0);
      }

      if (conn.recv(ret.data, size)) {
         if (conn.tookMultipleReads()) {
            printf("*** mutiple reads on data\n");
            printf("*** Strange packet, type: %d, size: %d\n", type, size);
            printf("*** Last packet was type: %d, size: %d\n", lastType, lastSize);
         }
         lastType = type;
         lastSize = size;
         return ret;
      }
   }

   
   return Packet(0);
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
      Packet p(position);
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
      Packet p(click);
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
      Packet p(message);
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
      Packet p(connect);
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
      Packet p(changePvp);
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
      Packet p(signal);
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
      Packet p(arrow);
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
      Packet p(initialize);
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
      Packet p(healthChange);
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
      Packet p(serverList);
      p.data.writeInt(uLongList.size());
      for(unsigned i = 0; i < uLongList.size(); i++){
         p.data.writeLong(uLongList.at(i));
      }
      return p;
   }
};


} // end pack namespace

#endif
