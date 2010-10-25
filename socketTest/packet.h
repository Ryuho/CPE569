#ifndef PACKET_HPP
#define PACKET_HPP

namespace packet {

/// Length of 12 bytes
struct info
{
  unsigned int playerID; // an int that uniquely identifies a player
  float x;      // absolute x coordinate of the player in the world
  float y;      // absolute y coordinate of the player in the world
};

}

#endif // PACKET_HPP

