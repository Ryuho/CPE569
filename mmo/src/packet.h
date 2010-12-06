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

} // end pack namespace

#endif
