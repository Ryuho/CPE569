#ifndef PACKET_HPP
#define PACKET_HPP

/// Structure to hold information about an event
struct packet
{
  std::string code;
  std::string name;
  double open_price;
  double high_price;
  double low_price;
  double last_price;
  double buy_price;
  int buy_quantity;
  double sell_price;
  int sell_quantity;

};


#endif // PACKET_HPP

