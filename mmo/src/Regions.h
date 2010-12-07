#ifndef _BASE_OBJECTS_H_
#define _BASE_OBJECTS_H_

#include <vector>
#include <map>

namespace regionManager {
   using namespace std;

   struct RMObject 
   {
      RMObject(int id) : id(id) {}
      int getId() const { return id; }
   protected:
      int id;
   };


   struct Region 
   {
      Region(int id, unsigned typeCount);

      int getId() const; //Region Id
      std::vector<RMObject *> &getObjects(int typeIndex);
      RMObject *get(int objectId, int typeIndex) const;
      bool contains(int objectId) const;
      bool contains(int objectId, int typeIndex) const;
      bool add(RMObject *object, int typeIndex);
      bool remove(int objectId, int typeIndex);
      unsigned objectCount() const;
      unsigned typeCount() const;
      unsigned count(int typeIndex) const;

   private:
      bool _contains(int objectId, int typeIndex, unsigned index) const;

      int id;
      //speeds up lookup/removal of a specific object
      std::map<int, unsigned> objectMap; //objectMap[objectId] -> X where objectList[typeIndex][X] is the Object
      //allows iteration of each Type of object
      std::vector<std::vector<RMObject *>> objectList; //objectList[typeIndex] -> list of Objects of given type
   };


   struct RegionManagerData
   {
      RegionManagerData() 
         : obj(0), objectListIndex(0) {}
      RegionManagerData(RMObject *obj, int objectListIndex, 
         std::vector<int> &regionIds)
         : obj(obj), objectListIndex(objectListIndex), regionIds(regionIds) {}

      RMObject *obj; //could be removed for memory use efficiency
      unsigned objectListIndex;
      std::vector<int> regionIds;
   };


   struct RegionManager 
   {
      typedef std::vector<RMObject *>::iterator iterator;
      typedef std::vector<RMObject *>::const_iterator const_iterator;

      RegionManager(unsigned regionCount, unsigned typeCount);
      bool contains(int objectId) const;
      bool contains(int objectId, int typeIndex) const;
      RMObject *getObject(int objectId) const;
      Region *getRegion(int regionIndex);
      bool addObject(RMObject *object, int typeIndex, std::vector<int> &regionIds);
      RMObject *removeObject(int objectId, int typeIndex);
      const RegionManagerData *getData(int objectId) const;
      unsigned regionCount() const;
      unsigned objectCount() const;
      unsigned typeCount() const;
      unsigned count(int typeIndex) const;
      std::vector<RMObject *> &operator[](int typeIndex);

   private:
      bool _contains(int objectId, int typeIndex, unsigned index) const;

      unsigned objectTotal;
      std::map<int, RegionManagerData> objectToRegionsMap; //objectToRegionsMap[objectId] -> data
      //redundant list of Region's objects for more simplistic traversal
      std::vector<std::vector<RMObject *>> objectList; //objectList[typeIndex] -> list of Objects of given type
      std::vector<Region> regions;
   };
}

#endif // _BASE_OBJECTS_H_