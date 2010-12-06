#ifndef _BASE_OBJECTS_H_
#define _BASE_OBJECTS_H_

#include <vector>
#include <map>

namespace objmanager {
   using namespace std;

   struct Object 
   {
      Object(unsigned id) 
         : id(id) {}
      unsigned getId() const { return id; }
   protected:
      unsigned id;
   };


   struct Region 
   {
      Region(unsigned id, unsigned typeCount);

      unsigned getId() const; //Region Id
      std::vector<Object *> &getObjects(unsigned typeIndex);
      Object *get(unsigned objectId, unsigned typeIndex) const;
      bool contains(unsigned objectId) const;
      bool add(Object *object, unsigned typeIndex);
      bool remove(unsigned objectId, unsigned typeIndex);
      unsigned objectCount() const;
      unsigned typeCount() const;
      unsigned count(unsigned typeIndex) const;

   private:
      unsigned id;
      //speeds up lookup/removal of a specific object
      std::map<unsigned, unsigned> objectMap; //objectMap[objectId] -> X where objectList[typeIndex][X] is the Object
      //allows iteration of each Type of object
      std::vector<std::vector<Object *>> objectList; //objectList[typeIndex] -> list of Objects of given type
   };


   struct RegionManagerData
   {
      RegionManagerData() 
         : obj(0), objectListIndex(0) {}
      RegionManagerData(Object *obj, unsigned objectListIndex, 
         std::vector<unsigned> &regionIds)
         : obj(obj), objectListIndex(objectListIndex), regionIds(regionIds) {}

      Object *obj; //could be removed for memory use efficiency
      unsigned objectListIndex;
      std::vector<unsigned> regionIds;
   };


   struct RegionManager 
   {
      typedef std::vector<objmanager::Object *>::iterator iterator;
      typedef std::vector<objmanager::Object *>::const_iterator const_iterator;

      RegionManager(unsigned regionCount, unsigned typeCount);
      Object *getObject(unsigned objectId);
      Region *getRegion(unsigned regionIndex);
      bool addObject(Object *object, unsigned typeIndex, std::vector<unsigned> &regionIds);
      Object *removeObject(unsigned objectId, unsigned typeIndex);
      const RegionManagerData *getData(unsigned objectId) const;
      unsigned regionCount() const;
      unsigned objectCount() const;
      unsigned typeCount() const;
      unsigned count(unsigned typeIndex) const;
      std::vector<Object *> &operator[](unsigned typeIndex);

   private:
      unsigned objectTotal;
      std::map<unsigned, RegionManagerData> objectToRegionsMap; //objectToRegionsMap[objectId] -> data
      //redundant list of Region's objects for more simplistic traversal
      std::vector<std::vector<Object *>> objectList; //objectList[typeIndex] -> list of Objects of given type
      std::vector<Region> regions;
   };
}

#endif // _BASE_OBJECTS_H_