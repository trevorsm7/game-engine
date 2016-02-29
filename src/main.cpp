#include "GlfwInstance.h"
#include "SDLInstance.h"
#include "Scene.h"

#include <cstdio>
#include <string>
#include <unistd.h>

bool findFile(const char* name, std::string& fullpath)
{
    static const char* paths[] = {"", "../assets/", "assets/", "../scripts/", "scripts/"};
    for (const char* path : paths)
    {
        auto temp = std::string(path) + name;
        if (access(temp.c_str(), R_OK) == 0)
        {
            fullpath = temp;
            return true;
        }
    }
    return false;
}

int main(int argc, char *argv[])
{
    const char* script = "snake.lua";
    bool useSDL = false;

    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "-sdl") == 0)
        {
            useSDL = true;
            continue;
        }

        script = argv[i];
    }

    std::string fullpath;
    if (!findFile(script, fullpath))
    {
        fprintf(stderr, "Unable to find file %s\n", script);
        return -1;
    }
    printf("Loading script %s\n", fullpath.c_str());

    if (useSDL)
        SdlInstance::run(fullpath.c_str());
    else
        GlfwInstance::run(fullpath.c_str());

    return 0;
}
