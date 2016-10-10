#include "SdlSample.hpp"

#include <SDL2/SDL.h>
#include <SDL2_mixer/SDL_mixer.h>

SdlSample::~SdlSample()
{
    if (m_sample)
        Mix_FreeChunk(m_sample);
}

void SdlSample::playSample()
{
    if (m_sample)
        Mix_PlayChannel(-1, m_sample, 0);
}

SdlSamplePtr SdlSample::loadSample(ResourceManager& manager, const std::string& filename)
{
    // Return the resource if it is cached
    // TODO should we check for special case where resource is loaded as a different type
    IResourcePtr resource = manager.getResource(filename);
    SdlSamplePtr sample = std::dynamic_pointer_cast<SdlSample>(resource);
    if (sample)
        return sample;

    Mix_Chunk* chunk = nullptr; // = Mix_LoadWAV(filename.c_str());
    std::vector<char> data;
    if (manager.loadRawData(filename, data))
    {
        SDL_RWops* rwops = SDL_RWFromMem(data.data(), data.size());
        chunk = Mix_LoadWAV_RW(rwops, 1);
        if (!chunk)
        {
            fprintf(stderr, "Error loading sample %s\n", SDL_GetError());
            //return sample; // TODO placeholder?
        }
    }

    sample = SdlSamplePtr(new SdlSample(chunk));

    // Cache the resource and return it
    manager.bindResource(filename, sample);
    return sample;
}
