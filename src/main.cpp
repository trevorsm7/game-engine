#include "Scene.hpp"

#include <cstdio>
#include <cassert>
#include <string>

#ifndef WIN32
#include <unistd.h>
#else
#include <Windows.h>
#include "Shlwapi.h"
#pragma comment(lib, "shlwapi.lib")
#endif

typedef void (*RunFunc)(const char*);
struct Platform {const char* const key; RunFunc func;};

#ifdef PLATFORM_SDL
#undef PLATFORM_SDL
#include "SDLInstance.hpp"
#define PLATFORM_SDL {"-sdl", SdlInstance::run},
#else
#define PLATFORM_SDL
#endif

#ifdef PLATFORM_GLFW
#undef PLATFORM_GLFW
#include "GlfwInstance.hpp"
#define PLATFORM_GLFW {"-glfw", GlfwInstance::run},
#else
#define PLATFORM_GLFW
#endif

static Platform platforms[] = {PLATFORM_SDL PLATFORM_GLFW {nullptr, nullptr}};

bool findPlatform(const char* key, RunFunc& func)
{
    Platform* entry = platforms;

    while (entry->key != nullptr)
    {
        if (strcmp(key, entry->key) == 0)
        {
            func = entry->func;
            return true;
        }

        ++entry;
    }

    return false;
}

bool findFile(const char* name, std::string& fullpath)
{
    static const char* paths[] = {"", "../assets/", "assets/", "../scripts/", "scripts/"};
    for (const char* path : paths)
    {
        auto temp = std::string(path) + name;
#ifndef WIN32
        if (access(temp.c_str(), R_OK) == 0)
#else
		if (PathFileExistsA(temp.c_str()) == TRUE)
#endif
        {
            fullpath = temp;
            return true;
        }
    }
    return false;
}

int main(int argc, char* argv[])
{
    const char* script = "breakout.lua";
    RunFunc platform = platforms[0].func;
    assert(platform != nullptr);

    for (int i = 1; i < argc; ++i)
    {
        if (findPlatform(argv[i], platform))
            continue;

        script = argv[i];
    }

    std::string fullpath;
    if (!findFile(script, fullpath))
    {
        fprintf(stderr, "Unable to find file %s\n", script);
        return -1;
    }

    fprintf(stderr, "Loading script %s\n", fullpath.c_str());
    platform(fullpath.c_str());

    return 0;
}
