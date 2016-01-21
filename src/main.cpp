#include "DebugRenderer.h"
#include "GlfwInstance.h"
#include "Scene.h"

#include <cstdio>
#include <string>
#include <unistd.h>

void debugMain(const char* script)
{
    DebugRenderer renderer;

    Scene scene;
    scene.load(script);
    for (int i = 0; i < 5; ++i)
    {
        scene.update(1.f);
        scene.render(&renderer);
    }
}

bool findFile(const char* name, std::string& fullpath)
{
    static const char* paths[] = {"./", "../assets/", "./assets/"};
    for (const char* path : paths)
    {
        fullpath = std::string(path) + name;
        if (access(fullpath.c_str(), R_OK) == 0)
            return true;
    }
    return false;
}

int main(int argc, char *argv[])
{
    const char* script = "test.lua";
    bool debug = false;

    if (argc > 1)
    {
        if (strcmp(argv[1], "-debug") == 0)
        {
            debug = true;
            if (argc > 2)
                script = argv[2];
        }
        else
            script = argv[1];
    }

    std::string fullpath;
    if (!findFile(script, fullpath))
    {
        fprintf(stderr, "Unable to find file %s\n", script);
        return -1;
    }
    printf("Loading script %s\n", fullpath.c_str());

    if (debug)
        debugMain(fullpath.c_str());
    else
        GlfwInstance::run(fullpath.c_str());

    return 0;
}
