#pragma once

#include "IResource.hpp"

#include <string>
#include <vector>
#include <map>

class ResourceManager
{
private:
    std::map<std::string, IResourcePtr> m_ptrMap;

public:
    ResourceManager() {}
    ~ResourceManager() {}

    IResourcePtr getResource(const std::string& name);
    bool loadRawData(const std::string& name, std::vector<char>& data);
    void bindResource(const std::string& name, IResourcePtr resource) {m_ptrMap[name] = resource;}
};
