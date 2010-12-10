#include "GameServer.h"
#include "Packets.h"
#include "ServerUtil.h"
#include "Constants.h"
#include <cstdio>

using namespace pack;
using namespace server;

//TODO: update this!!!
void GameServer::newClientConnection(int id)
{
   printf("New client connection: %d\n", id);
   
   vec2 pos((float)(rand()%200), (float)(rand()%200));
   
   Player *newPlayer 
      = new Player(id, cm.ownServerId, pos, vec2(0,1), playerMaxHp);
   om.add(newPlayer);

	clientSendPacket(Connect(id, constants::worldHeight, 
      constants::worldWidth), id);
   clientBroadcast(newPlayer->cserialize());

   sendPlayerInitData(id, om);
   sendPlayerInitData(id, som);
   serverBroadcast(newPlayer->serialize());
}

void GameServer::clientDisconnect(int id)
{
   printf("Client %d disconnected\n", id);
   Packet removePacket(Signal(Signal::remove, id).makePacket());
   clientBroadcast(removePacket);
   serverBroadcast(removePacket);
   om.remove(id);
}

//TODO: update as needed for multi-server
void GameServer::sendPlayerAOI(Player &p, ObjectHolder &oh)
{
   Geometry aoi(Circle(p.pos, areaOfInfluenceRadius));
   std::vector<NPCBase *> aoinpcs;
   oh.collidingNPCs(aoi, p.pos, aoinpcs);
   for(unsigned i = 0; i < aoinpcs.size(); i++) {
      NPC &npc = *static_cast<NPC *>(aoinpcs[i]);
      clientSendPacket(HealthChange(npc.getId(), npc.hp), p.getId());
      clientSendPacket(Position(npc.pos, npc.dir, npc.moving, npc.getId()), 
         p.getId());
   }
   std::vector<PlayerBase *> aoiplayers;
   oh.collidingPlayers(aoi, p.pos, aoiplayers);
   for(unsigned i = 0; i < aoiplayers.size(); i++) {
      Player &player = *static_cast<Player *>(aoiplayers[i]);
      clientSendPacket(HealthChange(player.getId(), player.hp), p.getId());
      if(player.getId() != p.getId()) {
         clientSendPacket(Position(player.pos, player.dir, player.moving, 
            player.getId()),  p.getId());
      }
   }
}

void updateServers()
{
   //tell new server about previous players (includes self)
   for(unsigned i = 0; i < getOM().playerCount(); i++) {
      Player &obj = *static_cast<Player *>(getOM().get(ObjectType::Player, i));
      getGS().serverBroadcast(obj.serialize());
   }
   //tell new server about previous Items
   for(unsigned i = 0; i < getOM().itemCount(); i++) {
      Item &obj = *static_cast<Item *>(getOM().get(ObjectType::Item, i));
      getGS().serverBroadcast(obj.serialize());
   }
   //tell new server about previous NPCs
   for(unsigned i = 0; i < getOM().npcCount(); i++) {
      NPC &obj = *static_cast<NPC *>(getOM().get(ObjectType::NPC, i));
      getGS().serverBroadcast(obj.serialize());
   }
   //tell new servers about previous Missiles
   //for(unsigned i = 0; i < getOM().missileCount(); i++) {
   //   Missile &obj = *static_cast<Missile *>(getOM().get(ObjectType::Missile, i));
   //   getGS().serverBroadcast(obj.serialize());
   //}
}

void GameServer::removePlayer(Player &p)
{
   clientBroadcast(Signal(Signal::remove, p.getId()));
   serverBroadcast(Signal(Signal::remove, p.getId()));
   removeClientConnection(p.getId());
   om.remove(p.getId());
}

void GameServer::removeObject(ObjectBase &obj)
{
   int id = obj.getId();
   clientBroadcast(Signal(Signal::remove, id));
   serverBroadcast(Signal(Signal::remove, id));
   if(om.contains(id)){
      om.remove(id);
   }
   else if(som.contains(id)){
      som.remove(id);
   }
   else{
      printf("Ryuho: trying to remove something that is not in om or som\n");
   }
}

void GameServer::update(int ticks)
{
   //get the current delta time (time passed since it last ran update())
   dt = (ticks - this->ticks)/1000.0f;
   this->ticks = ticks;

   updatePlayers(ticks, dt);
   updateNPCs(ticks, dt);
   updateMissiles(ticks, dt); //updated last to ensure near monsters are hit
   if(rsid < 0) {
      //if there is a player connected, spawn NPCs, evenly distributed
      int npcCount = om.npcCount() + som.npcCount();
      if(npcCount < constants::npcQuantity) {
         for(unsigned i = 0; i < regionXSize; i++) {
            for(unsigned j = 0; j < regionYSize; j++) {
               NPC *npc = spawnNPC(i, j);
               //clientBroadcast(npc->cserialize());
            }
         }
      }
   }
   updateServers();
   totalUpdates++;
}

void GameServer::sendPlayerArrow(Player &player, vec2 dir)
{
   player.shotThisFrame = true;
   Missile *m = new Missile(newId(), cm.ownServerId, player.getId(), player.pos, 
      dir);
   createMissile(m);
}

void GameServer::createObject(ObjectBase *obj)
{
   om.add(obj);
   clientBroadcast(om.getCSerialized(obj->getId()));
}

void GameServer::createMissile(Missile *m)
{
   createObject(m);
   serverBroadcast(om.getSerialized(m->getId()));
}

////////////////////////////////////////////
/////////// Multi Server Methods ///////////
////////////////////////////////////////////
void GameServer::newServerConnection(int id)
{
   printf("New server connection: %d\n", id);
}

void GameServer::serverDisconnect(int id)
{
   printf("Server %d disconnected\n", id);
   int newId = -1;

   for(int i = id+1; i < cm.nextServId; i++){
      if(newId != -1){
         break;
      }
      if(cm.idToServerIndex.find(i) != cm.idToServerIndex.end() || i == cm.ownServerId){
         newId = i;
      }
   }
   for(int i = 0; i < id; i++){
      if(cm.idToServerIndex.find(i) != cm.idToServerIndex.end() || i == cm.ownServerId){
         newId = i;
      }
      if(newId != -1){
         break;
      }
   }

   if(newId == -1){
      printf("A valid server ID could not be found! This is impossible because THIS server is alive!!\n");
      printf("should have deleted %d NPCs that was on the server that died.\n",som.npcCount());
      printf("should have deleted %d Items that was on the server that died.\n",som.itemCount());
      return;
   }


   //FIXME need to get the object from som, change the id, then add it to om
   //i think i changed the id, and added it to om, but i can't remove it from som,
   //and I don't know if removing it from om would be bad because it's a pointer
   for(unsigned i = 0; i < som.npcCount(); i++){
      NPC *obj = new NPC(*som.getNPCByIndex(i));
      if(obj->sid == id) {
         obj->sid = newId;
         om.add(obj);
         som.remove(obj->getId());
         //printf("changed NPC# %d to be owned by server %d\n",curr->getId(),newId);
      }
   }
   for(unsigned i = 0; i < som.itemCount(); i++){
      Item *obj = new Item(*som.getItemByIndex(i));
      if(obj->sid == id) {
         obj->sid = newId;
         om.add(obj);
         som.remove(obj->getId());
         //printf("changed Item# %d to be owned by server %d\n",curr->getId(),newId);
      }
   }
}

void GameServer::serverSendPacket(Packet &p, int id)
{
   cm.serverSendPacket(p, id);
}

void GameServer::serverBroadcast(Packet &p)
{
   cm.serverBroadcast(p);
}

void GameServer::processServerPacket(pack::Packet p, int id)
{
   if(p.type == PacketType::serialPlayer) {
      Player obj(p);
      if(!som.contains(obj.getId())) {
         printf("Ryuho: A foreign server added a player to this server\n");
         som.add(new Player(obj));
         clientBroadcast(obj.cserialize());
      }
      else {
         Player *obj2 = som.getPlayer(obj.getId());
         *obj2 = obj;
      }
      if(om.contains(obj.getId())){
         printf("Ryuho: A foreign server tried to access player owned by this server\n");
      }
   }
   else if(p.type == PacketType::serialItem) {
      Item obj(p);
      if(!som.contains(obj.getId())) {
         som.add(new Item(obj));
         clientBroadcast(obj.cserialize());
      }
      else {
         Item *obj2 = som.getItem(obj.getId());
         *obj2 = obj;
      }
      if(om.contains(obj.getId())){
         printf("Ryuho: A foreign server tried to access Item owned by this server\n");
      }
   }
   else if(p.type == PacketType::serialMissile) {
      Missile obj(p);
      if(!som.contains(obj.getId())) {
         som.add(new Missile(obj));
         clientBroadcast(obj.cserialize());
      }
      //else {
         //Missile *obj2 = som.getMissile(obj.getId());
         //*obj2 = obj;
      //}
      if(om.contains(obj.getId())){
         printf("Ryuho: A foreign server tried to access missle owned by this server\n");
      }
   }
   else if(p.type == PacketType::serialNPC) {
      NPC obj(p);
      if(!som.contains(obj.getId())) {
         som.add(new NPC(obj));
         clientBroadcast(obj.cserialize());
      }
      else {
         NPC *obj2 = som.getNPC(obj.getId());
         *obj2 = obj;
      }
      if(om.contains(obj.getId())){
         printf("Ryuho: A foreign server tried to access NPC owned by this server\n");
      }
   }
   else if(p.type == PacketType::signal) {
      Signal signal(p);
      if(signal.sig == Signal::remove) {
         if(som.contains(signal.val)) {
            som.remove(signal.val);
            clientBroadcast(signal);
            //may still remove other server's objects... and no easy way to check
         }
         else if(om.contains(signal.val)) {
            om.remove(signal.val);
            clientBroadcast(signal);
            printf("Server %d requesting remote removal of %d\n", 
               id, signal.val);
         }
      }
      else
         printf("Error: unknown signal (sig=%d val=%d)\n", 
            signal.sig, signal.val);
   }
   else
      printf("Error: unknown server packet, size: %d\n", p.data.size());
}