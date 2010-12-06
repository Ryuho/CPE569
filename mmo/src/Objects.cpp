#include "Objects.h"
#include <assert.h>

using namespace std;
using namespace objmanager;


///////////////////////////////////////
//////////////// Region ///////////////
///////////////////////////////////////
Region::Region(unsigned id, unsigned typeCount)
   : id(id)
{
   for(unsigned i = 0; i < typeCount; i++) {
      objectList.push_back(std::vector<Object *>());
   }
}

unsigned Region::getId() const
{
   return id;
}

std::vector<Object *> &Region::getObjects(unsigned typeIndex)
{
   assert(typeIndex < typeCount());
   return objectList[typeIndex];
}

Object *Region::get(unsigned objectId, unsigned typeIndex) const
{
   assert(typeIndex < typeCount());
 
   std::map<unsigned, unsigned>::const_iterator iter
      = objectMap.find(objectId);
   if(iter == objectMap.end()) {
      return 0; //not found
   }
   return objectList[typeIndex][iter->second];
}

bool Region::contains(unsigned objectId) const
{
   return objectMap.find(objectId) != objectMap.end();
}

bool Region::add(Object *object, unsigned typeIndex)
{
   assert(typeIndex < typeCount());

   std::map<unsigned, unsigned>::iterator iter
      = objectMap.find(object->getId());
   if(iter != objectMap.end()) {
      return false; //already exists
   }
   objectMap[object->getId()] = objectList[typeIndex].size();
   objectList[typeIndex].push_back(object);
   return true;
}

bool Region::remove(unsigned objectId, unsigned typeIndex)
{
   assert(typeIndex < typeCount());

   std::map<unsigned, unsigned>::iterator iter
      = objectMap.find(objectId);
   if(iter == objectMap.end()) {
      return false; //doesn't exist
   }
   //1. swap in list with end of list
   //2. replace old end of list's mapped index
   //3. remove object from map AND list
   unsigned index = objectMap[objectId];
   //swap removed with end of list
   Object *replacement = objectList[typeIndex][objectList[typeIndex].size() - 1];
   objectList[typeIndex][index] = replacement;
   objectList[typeIndex].pop_back();
   //fix replacement's index in objectMap
   objectMap[replacement->getId()] = index;
   //remove from objectMap
   objectMap.erase(objectId);
   return true;
}

unsigned Region::objectCount() const
{
   return objectMap.size();
}

unsigned Region::typeCount() const
{
   return objectList.size();
}

unsigned Region::count(unsigned typeIndex) const
{
   assert(typeIndex < typeCount());
   return objectList[typeIndex].size();
}

///////////////////////////////////////
//////////// RegionManager ////////////
///////////////////////////////////////
RegionManager::RegionManager(unsigned regionCount, unsigned typeCount)
   : objectTotal(0)
{
   for(unsigned i = 0; i < regionCount; i++) {
      regions.push_back(Region(i, typeCount));
   }
   for(unsigned i = 0; i < typeCount; i++) {
      objectList.push_back(std::vector<Object *>());
   }
}

Object *RegionManager::getObject(unsigned objectId)
{
   std::map<unsigned, RegionManagerData>::iterator iter;

   iter = objectToRegionsMap.find(objectId);
   if(iter == objectToRegionsMap.end()) {
      return 0; //doesn't exist
   }
   return iter->second.obj;
}

Region *RegionManager::getRegion(unsigned regionIndex)
{
   assert(regionIndex < regionCount());
   return &regions[regionIndex];
}

bool RegionManager::addObject(Object *object, unsigned typeIndex,
      std::vector<unsigned> &regionIds)
{
   assert(typeIndex < typeCount());
 
   if(objectToRegionsMap.find(object->getId()) != objectToRegionsMap.end()) {
      return false; //already exists
   }
   //add to Regions
   for(unsigned i = 0; i < regionIds.size(); i++) {
      regions[regionIds[i]].add(object, typeIndex);
   }
   //add to List
   objectList[typeIndex].push_back(object);
   //add to Map Data
   objectToRegionsMap[object->getId()]
      = RegionManagerData(object, objectList[typeIndex].size()-1, regionIds);
   objectTotal++;

   return true;
}

Object *RegionManager::removeObject(unsigned objectId, unsigned typeIndex)
{
   assert(typeIndex < typeCount());

   std::map<unsigned, RegionManagerData>::iterator iter;
   iter = objectToRegionsMap.find(objectId);
   if(iter == objectToRegionsMap.end()) {
      return 0; //doesn't exists
   }
   Object *obj = iter->second.obj;
   //remove from Regions
   std::vector<unsigned> &regionIds = iter->second.regionIds;
   for(unsigned i = 0; i < regionIds.size(); i++) {
      regions[regionIds[i]].remove(objectId, typeIndex);
   }
   //remove from List
   unsigned currIndex = iter->second.objectListIndex;
   int replacementIndex = objectList[typeIndex].size()-1;
   Object *replacement = objectList[typeIndex][replacementIndex];
   objectList[typeIndex][currIndex] = replacement;
   objectList[typeIndex].pop_back();
   //remove from Map Data
   objectToRegionsMap[replacement->getId()].objectListIndex = currIndex;
   objectToRegionsMap.erase(objectId);

   objectTotal--;
   return obj;
}

const RegionManagerData *RegionManager::getData(unsigned objectId) const
{
   std::map<unsigned, RegionManagerData>::const_iterator iter
      = objectToRegionsMap.find(objectId);
   if(iter == objectToRegionsMap.end())
      return 0;
   return &iter->second;
}

/////////// Count ///////////
unsigned RegionManager::regionCount() const
{
   return regions.size();
}

unsigned RegionManager::objectCount() const
{
   return objectTotal;
}

unsigned RegionManager::typeCount() const
{
   return objectList.size();
}

unsigned RegionManager::count(unsigned typeIndex) const
{
   assert(typeIndex < objectList.size());
   return objectList[typeIndex].size();
}

std::vector<Object *> &RegionManager::operator[](unsigned typeIndex)
{
   assert(typeIndex < objectList.size());
   return objectList[typeIndex];
}