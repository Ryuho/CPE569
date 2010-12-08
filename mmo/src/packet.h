#ifndef _PACKET_H_
#define _PACKET_H_

#include <boost/shared_ptr.hpp>
#include "socket.h"
#include "matrix.h"
#include <string>
#include <stdio.h>

namespace pack {
using namespace mat;

// Simple structure for reading/writing using sockets.
struct Packet {
   Packet(int type) : type(type) {}
   sock::Packet data;
   int type;
   
   bool sendTo(sock::Connection conn) {
      sock::Packet header;
      header.writeInt(data.size()).writeInt(type);
      bool val = conn.send(header) && conn.send(data);
      //if(!val)
      //   printf("sendTo: failure\n");
      return val;
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

//Can be used to create a list or tree of Packets 
//e.g. nest Packet2s within a Packet2
struct Packet2 {
   Packet2() {}
   Packet2(Packet &p)  {
      unsigned count;
      p.data.readUInt(count);
      for(unsigned i = 0; i < count; i++) {
         unsigned count2;
         int subtype;
         p.data.readInt(subtype);
         p.data.readUInt(count2);
         Packet pn(subtype);
         for(unsigned j = 0; j < count2; j++) {
            unsigned char byt;
            p.data.readByte(byt);
            pn.data.writeByte(byt);
         }
         pn.data.setCursor(0);
         packets.push_back(pn);
      }
      p.data.reset();
   }

   Packet makePacket() {
      Packet p(pack2type);
      p.data.writeUInt(packets.size());
      for(unsigned i = 0; i < packets.size(); i++) {
         p.data.writeInt(packets[i].type);
         p.data.writeUInt(packets[i].data.size());
         p.data.writeData(&packets[i].data[0], packets[i].data.size());
      }
      return p;
   }
   static const int pack2type = 0;
   std::vector<pack::Packet> packets;
};

} // end pack namespace

#endif
