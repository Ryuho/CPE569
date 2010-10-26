#ifndef _PACKET_H_
#define _PACKET_H_

#include <boost/shared_ptr.hpp>
#include "socket.h"
#include "matrix.h"
#include <string>

namespace pack {

enum PacketType {
   pos = 1,
   message = 2,
   connect = 3,
   signal = 4,
   // Add new packet types here, make values explicit
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
Packet readPacket(sock::Connection conn) {
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
struct Pos {
   mat::vec2 v;
   int id;
   Pos() {}
   Pos(mat::vec2 v) : v(v), id(0) {}
   Pos(mat::vec2 v, int id) : v(v), id(id) {}
   Pos(Packet &p) {
      p.data.readInt(id).readFloat(v.x).readFloat(v.y).reset();
   }
   Packet makePacket() {
      Packet p(12, pos);
      p.data.writeInt(id).writeFloat(v.x).writeFloat(v.y);
      return p;
   }
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
   Connect() : id(0) {}
   Connect(int id) : id(id) {}
   Connect(Packet &p) {
      p.data.readInt(id).reset();
   }
   Packet makePacket() {
      Packet p(4, connect);
      p.data.writeInt(id);
      return p;
   }
};

// Used to combine all simple signals into one packet
struct Signal {
   enum { 
      hello = 1,      // Not used yet... A simple ping
      disconnect = 2  // Indicates the player with id in val disconnected
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

// An example of how to use packets
void example(sock::Connection conn) {
   Packet p = readPacket(conn);
   
   if (p.type == pos) {
      Pos myPos(p);
      myPos.v.x = myPos.v.y;
   } else if (p.type == message) {
      Message myMsg(p);
      //printf("%s\n", myMsg.str.c_str());
   } else if (p.type == 0) {
      // Error reading from connection.
   } else {
      // Unknown packet type. Debug.
   }
}

} // end pack namespace

#endif
