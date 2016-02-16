#include "ResourceManager.h"

// TODO: this is a quick hack; ref to a hack in main.cpp
bool findFile(const char* name, std::string& fullpath);

IResourcePtr ResourceManager::getResource(const std::string& name)
{
    // Return ptr if resource already bound
    // NOTE: as currently implemented, this can return nullptr if load failed
    auto ptrIt = m_ptrMap.find(name);
    if (ptrIt != m_ptrMap.end())
        return ptrIt->second;

    // Get file extension
    size_t index = name.find_last_of('.');
    std::string ext = name.substr(index + 1);

    IResourcePtr ptr;

    // TODO: refer to main.cpp; should move this logic here
    std::string fullname;
    if (!findFile(name.c_str(), fullname))
    {
        fprintf(stderr, "Unable to find file \"%s\"\n", name.c_str());
    }
    else
    {
        // Find the resource loader for the extension
        auto loaderIt = m_loaderMap.find(ext);
        if (loaderIt == m_loaderMap.end())
            fprintf(stderr, "No resource loader for file \"%s\"\n", name.c_str());
        else
            ptr = loaderIt->second(fullname);
    }

    m_ptrMap[name] = ptr;
    return ptr;
}

/*ResourceId ResourceManager::bindResource(std::string name)
{
    // Return ID if resource already bound
    auto idIt = m_idMap.find(name);
    if (idIt != m_idMap.end())
        return idIt->second;

    // Get file extension
    size_t index = name.find_last_of('.');
    std::string ext = name.substr(index + 1);

    // Find the resource loader for the extension
    IResourcePtr ptr;
    auto loaderIt = m_loaderMap.find(ext);
    if (loaderIt == m_loaderMap.end())
        fprintf("No resource loader for file \"%s\"\n", name.c_str());
    else
        ptr = loaderIt->second(name); // call the resource loader
    //{
    //    ResourceLoader loader = it->second;
    //    std::vector data = archive->getFile(name);
    //    loader(data);
    //}

    // Create an entry for the resource
    ResourceId id = m_nextId++;
    m_nameMap[id] = name;
    m_ptrMap[id] = ptr; // let this be null if we failed to load
    m_idMap[name] = id;
}*/
