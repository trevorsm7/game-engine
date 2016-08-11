#include "ResourceManager.hpp"

#include <fstream>

// TODO: this is a quick hack; ref to a hack in main.cpp
bool findFile(const char* name, std::string& fullpath);

IResourcePtr ResourceManager::getResource(const std::string& name)
{
    IResourcePtr ptr;

    // Copy the pointer if we've already bound it
    auto it = m_ptrMap.find(name);
    if (it != m_ptrMap.end())
        ptr = it->second;

    return ptr;
}

bool ResourceManager::loadRawData(const std::string& name, std::vector<char>& data)
{
    // TODO: refer to main.cpp; should move this logic here
    // Find the file among our allowed search paths
    std::string fullname;
    if (!findFile(name.c_str(), fullname))
    {
        fprintf(stderr, "Unable to find file \"%s\"\n", name.c_str());
        return false;
    }

    // Open the file
    std::fstream file;
    file.open(fullname, file.in | file.binary);
    if (!file.is_open())
    {
        fprintf(stderr, "Unable to open file \"%s\"\n", fullname.c_str());
        return false;
    }

    // Get the length of the file
    file.seekg(0, file.end);
    int length = file.tellg();
    file.seekg(0, file.beg);

    // Read the image data
    data.resize(length);
    file.read(data.data(), length);
    file.close();

    return true;
}
