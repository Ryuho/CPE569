#ifndef _PACKET_H_
#define _PACKET_H_

#include <boost/shared_ptr.hpp>
#include "socket.h"
#include <string>

namespace pack {

enum PacketType {
   pos = 1,
   message = 2,
   // Add new packet types here, make values explicit
};

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

// Actual packet types

// You can even declare these somewhere else, just put a value in the enum

struct Pos {
   float x, y, z;
   Pos() {}
   Pos(float x, float y, float z) : x(x), y(y), z(z) {}
   Pos(Packet &p) {
      p.data.readFloat(x).readFloat(y).readFloat(z).reset();
   }
   Packet makePacket() {
      Packet p(12, pos);
      p.data.writeFloat(x).writeFloat(y).writeFloat(z);
      return p;
   }
};

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

// An example of how to use 

void example(sock::Connection conn) {
   Packet p = readPacket(conn);
   
   if (p.type == pos) {
      Pos myPos(p);
      myPos.x = myPos.y;
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
