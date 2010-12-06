#ifndef _BASE_OBJECTS_H_
#define _BASE_OBJECTS_H_

#include <vector>
#include <map>

namespace objmanager {
   using namespace std;

   struct Object 
   {
      Object(int id) : id(id) {}
      int getId() const { return id; }
   protected:
      int id;
   };


   struct Region 
   {
      Region(int id, unsigned typeCount);

      int getId() const; //Region Id
      std::vector<Object *> &getObjects(int typeIndex);
      Object *get(int objectId, int typeIndex) const;
      bool contains(int objectId) const;
      bool add(Object *object, int typeIndex);
      bool remove(int objectId, int typeIndex);
      unsigned objectCount() const;
      unsigned typeCount() const;
      unsigned count(int typeIndex) const;

   private:
      int id;
      //speeds up lookup/removal of a specific object
      std::map<int, int> objectMap; //objectMap[objectId] -> X where objectList[typeIndex][X] is the Object
      //allows iteration of each Type of object
      std::vector<std::vector<Object *>> objectList; //objectList[typeIndex] -> list of Objects of given type
   };


   struct RegionManagerData
   {
      RegionManagerData() 
         : obj(0), objectListIndex(0) {}
      RegionManagerData(Object *obj, int objectListIndex, 
         std::vector<int> &regionIds)
         : obj(obj), objectListIndex(objectListIndex), regionIds(regionIds) {}

      Object *obj; //could be removed for memory use efficiency
      int objectListIndex;
      std::vector<int> regionIds;
   };


   struct RegionManager 
   {
      typedef std::vector<objmanager::Object *>::iterator iterator;
      typedef std::vector<objmanager::Object *>::const_iterator const_iterator;

      RegionManager(unsigned regionCount, unsigned typeCount);
      Object *getObject(int objectId);
      Region *getRegion(int regionIndex);
      bool addObject(Object *object, int typeIndex, std::vector<int> &regionIds);
      Object *removeObject(int objectId, int typeIndex);
      const RegionManagerData *getData(int objectId) const;
      unsigned regionCount() const;
      unsigned objectCount() const;
      unsigned typeCount() const;
      unsigned count(int typeIndex) const;
      std::vector<Object *> &operator[](int typeIndex);

   private:
      unsigned objectTotal;
      std::map<int, RegionManagerData> objectToRegionsMap; //objectToRegionsMap[objectId] -> data
      //redundant list of Region's objects for more simplistic traversal
      std::vector<std::vector<Object *>> objectList; //objectList[typeIndex] -> list of Objects of given type
      std::vector<Region> regions;
   };
}

#endif // _BASE_OBJECTS_H_