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
   std::map<int, unsigned>::const_iterator iter = objectMap.find(objectId);
   if(iter == objectMap.end()) {
      return 0; //doesn't exist
   }
   unsigned index = iter->second;
   if(_contains(objectId, typeIndex, index)) 
      return 0; //invalid type, but exists
   return objectList[typeIndex][index];
}

bool Region::contains(int objectId) const
{
   return objectMap.find(objectId) != objectMap.end();
}

bool Region::contains(int objectId, int typeIndex) const
{
   return get(objectId, typeIndex) != 0;
}

bool Region::add(RMObject *object, int typeIndex)
{
   std::map<int, unsigned>::iterator iter = objectMap.find(object->getId());
   if(iter != objectMap.end()) {
      return false; //already exists
   }
   objectMap[object->getId()] = objectList[typeIndex].size();
   objectList[typeIndex].push_back(object);
   return true;
}

bool Region::_contains(int objectId, int typeIndex, unsigned index) const
{
   return objectList[typeIndex].size() > index 
      && objectList[typeIndex][index]->getId() == objectId;
}

bool Region::remove(int objectId, int typeIndex)
{
   std::map<int, unsigned>::iterator iter = objectMap.find(objectId);
   if(iter == objectMap.end()) {
      return false; //doesn't exist
   }
   unsigned index = objectMap[objectId];
   if(!_contains(objectId, typeIndex, index))
      return false; //invalid type, but exists
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

RMObject *RegionManager::getObject(int objectId) const
{
   std::map<int, RegionManagerData>::const_iterator iter;

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
   unsigned currIndex = iter->second.objectListIndex;
   if(!_contains(objectId, typeIndex, currIndex))
      return false; //wrong type, but exists

   RMObject *obj = iter->second.obj;
   //remove from Regions
   std::vector<int> &regionIds = iter->second.regionIds;
   for(unsigned i = 0; i < regionIds.size(); i++) {
      regions[regionIds[i]].remove(objectId, typeIndex);
   }
   //remove from List
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

bool RegionManager::_contains(int objectId, int typeIndex, unsigned index) const
{
   return objectList[typeIndex].size() > index
      && objectList[typeIndex][index]->getId() == objectId;
}

bool RegionManager::contains(int objectId) const
{
   return objectToRegionsMap.find(objectId) != objectToRegionsMap.end();
}

bool RegionManager::contains(int objectId, int typeIndex) const
{
   std::map<int, RegionManagerData>::const_iterator iter
      = objectToRegionsMap.find(objectId);
   if(iter == objectToRegionsMap.end())
      return false;
   return _contains(objectId, typeIndex, iter->second.objectListIndex);
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