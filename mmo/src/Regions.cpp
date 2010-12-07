#include "Regions.h"

using namespace std;
using namespace regionManager;


///////////////////////////////////////
//////////////// Region ///////////////
///////////////////////////////////////
Region::Region(int id, unsigned typeCount)
   : id(id)
{
   for(unsigned i = 0; i < typeCount; i++) {
      objectList.push_back(std::vector<RMObject *>());
   }
}

int Region::getId() const
{
   return id;
}

std::vector<RMObject *> &Region::getObjects(int typeIndex)
{
   return objectList[typeIndex];
}

RMObject *Region::get(int objectId, int typeIndex) const
{
   std::map<int, int>::const_iterator iter = objectMap.find(objectId);
   if(iter == objectMap.end()) {
      return 0; //not found
   }
   return objectList[typeIndex][iter->second];
}

bool Region::contains(int objectId) const
{
   return objectMap.find(objectId) != objectMap.end();
}

bool Region::add(RMObject *object, int typeIndex)
{
   std::map<int, int>::iterator iter = objectMap.find(object->getId());
   if(iter != objectMap.end()) {
      return false; //already exists
   }
   objectMap[object->getId()] = objectList[typeIndex].size();
   objectList[typeIndex].push_back(object);
   return true;
}

bool Region::remove(int objectId, int typeIndex)
{
   std::map<int, int>::iterator iter = objectMap.find(objectId);
   if(iter == objectMap.end()) {
      return false; //doesn't exist
   }
   //1. swap in list with end of list
   //2. replace old end of list's mapped index
   //3. remove object from map AND list
   int index = objectMap[objectId];
   //swap removed with end of list
   RMObject *replacement = objectList[typeIndex][objectList[typeIndex].size() - 1];
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

unsigned Region::count(int typeIndex) const
{
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
      objectList.push_back(std::vector<RMObject *>());
   }
}

RMObject *RegionManager::getObject(int objectId)
{
   std::map<int, RegionManagerData>::iterator iter;

   iter = objectToRegionsMap.find(objectId);
   if(iter == objectToRegionsMap.end()) {
      return 0; //doesn't exist
   }
   return iter->second.obj;
}

Region *RegionManager::getRegion(int regionIndex)
{
   return &regions[regionIndex];
}

bool RegionManager::addObject(RMObject *object, int typeIndex,
   std::vector<int> &regionIds)
{
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

RMObject *RegionManager::removeObject(int objectId, int typeIndex)
{
   std::map<int, RegionManagerData>::iterator iter;
   iter = objectToRegionsMap.find(objectId);
   if(iter == objectToRegionsMap.end()) {
      return 0; //doesn't exists
   }
   RMObject *obj = iter->second.obj;
   //remove from Regions
   std::vector<int> &regionIds = iter->second.regionIds;
   for(unsigned i = 0; i < regionIds.size(); i++) {
      regions[regionIds[i]].remove(objectId, typeIndex);
   }
   //remove from List
   int currIndex = iter->second.objectListIndex;
   int replacementIndex = objectList[typeIndex].size()-1;
   RMObject *replacement = objectList[typeIndex][replacementIndex];
   objectList[typeIndex][currIndex] = replacement;
   objectList[typeIndex].pop_back();
   //remove from Map Data
   objectToRegionsMap[replacement->getId()].objectListIndex = currIndex;
   objectToRegionsMap.erase(objectId);

   objectTotal--;
   return obj;
}

const RegionManagerData *RegionManager::getData(int objectId) const
{
   std::map<int, RegionManagerData>::const_iterator iter
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

unsigned RegionManager::count(int typeIndex) const
{
   return objectList[typeIndex].size();
}

std::vector<RMObject *> &RegionManager::operator[](int typeIndex)
{
   return objectList[typeIndex];
}