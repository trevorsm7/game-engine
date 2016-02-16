#ifndef __RESOURCEMANAGER_H__
#define __RESOURCEMANAGER_H__

#include "IResource.h"

#include <string>
#include <map>

//typedef unsigned ResourceId;
// TODO: probably want loader to take a blob of data rather than to open file directly
typedef IResourcePtr (*ResourceLoader)(std::string);

class ResourceManager
{
private:
    std::map<std::string, ResourceLoader> m_loaderMap;
    // NOTE: could combine name and ptr in one map
    //std::map<ResourceId, struct{std::string name; IResourcePtr ptr;}>
    // in the future, we may also want other members like last frame used
    //std::map<ResourceId, std::string> m_nameMap;
    //std::map<ResourceId, IResourcePtr> m_ptrMap;
    //std::map<std::string, ResourceID> m_idMap;
    //ResourceID m_nextId;
    std::map<std::string, IResourcePtr> m_ptrMap;

public:
    //ResourceManager(): m_nextId(0) {}
    ~ResourceManager() {}

    void addLoader(ResourceLoader loader, const char* ext) {m_loaderMap[ext] = loader;}

    IResourcePtr getResource(const std::string& name);
    //ResourceId bindResource(std::string name);
    //IResourcePtr getResource(ResourceId id) {return m_ptrMap[id];} // TODO: this should reload the resource if it was freed
    //std::string getName(ResourceId id) {return m_nameMap[id];}
};

#endif
