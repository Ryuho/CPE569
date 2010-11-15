#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <boost/shared_ptr.hpp>
#include <vector>

namespace sock {
   void setupSockets();
   void shutdownSockets();

   struct ConnectionInfo;
   struct ServerInfo;
   struct SelectInfo;

   class Socket {
   public:
      virtual unsigned long getSocket() const =0;
      virtual bool select(int ms = 0);
      virtual operator bool() const =0;
      virtual bool check() =0;
      virtual bool operator== (const Socket &other)
      {
         return this->getSocket() == other.getSocket();
      }
   protected:
      virtual void setError() =0;
   };

   class Packet : public std::vector<unsigned char> {
   public:
      Packet(int size=0) 
         : std::vector<unsigned char>(size), cursor(0), bitCursor(0) {}

      Packet &writeBit(bool b);
      Packet &writeByte(unsigned char b);
      Packet &writeShort(unsigned short s);
      Packet &writeInt(int i);
      Packet &writeLong(unsigned long l);
      Packet &writeFloat(float f);
      Packet &writeCStr(const char *str);
      Packet &writeStdStr(const std::string &str);
      Packet &writeData(const unsigned char *data, int length);

      Packet &readBit(bool &b);
      Packet &readByte(unsigned char &b);
      Packet &readShort(unsigned short &s);
      Packet &readInt(int &i);
      Packet &readLong(unsigned long &l);
      Packet &readFloat(float &f);
      Packet &readCStr(char *str);
      Packet &readStdStr(std::string &str);
      Packet &readData(unsigned char *data, int length);

      Packet &reset(int size = 0);
      Packet &setCursor(int pos);

   protected:
      int cursor, bitCursor;
      void checkSpace(int amount);
   };

   class Connection : public Socket {
   public:
      Connection() {}
      Connection(const char *host, int port);
      Connection(ConnectionInfo *info);
      bool send(const Packet &p);
      bool recv(Packet &p, int size = -1);
      void close();
      operator bool() const;
      unsigned long available();
      virtual unsigned long getSocket() const;
      virtual bool check();
      unsigned long getAddr();
      int getPort();
   protected:
      virtual void setError();
      boost::shared_ptr<ConnectionInfo> info;
   };

   class Server : public Socket {
   public:
      Server(int port = 0, const char *addr = 0);
      void listen(int queueSize);
      Connection accept();
      int port();
      void close();
      virtual unsigned long getSocket() const;
      virtual bool check();
      virtual operator bool() const;
   protected:
      virtual void setError();
      boost::shared_ptr<ServerInfo> info;
   };

   /*class SelectSet {
   public:
      SelectSet();
      void add(Connection c);
      void remove(Connection c);
      std::vector<Connection> select(int ms = 0);
      std::vector<Connection> nbRead(int ms = 0);
      void removeDisconnects();
   private:
      boost::shared_ptr<SelectInfo> info;
   };*/

   struct SelectSet {
      SelectSet();
      void add(int sd);
      void remove(int id);
      std::vector<int> select(int ms = 0);

      boost::shared_ptr<SelectInfo> info;
   };
}

#endif
