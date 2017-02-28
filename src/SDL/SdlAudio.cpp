#include "SdlAudio.hpp"

#include "SDL.h"
#include "SDL_mixer.h"

SdlSample::~SdlSample()
{
    if (m_sample)
        Mix_FreeChunk(m_sample);
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
        SDL_RWops* rwops = SDL_RWFromMem(data.data(), int(data.size()));
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

SdlAudio::~SdlAudio()
{
    // TODO need to stop all audio first?

    Mix_CloseAudio();
}

bool SdlAudio::init()
{
    /*int loaded = Mix_Init(MIX_INIT_MP3);
    if (loaded & MIX_INIT_MP3 == 0)
    {
        fprintf(stderr, "Failed to init MP3 audio\n");
    }*/

    // Set up the audio stream
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 512) != 0)
    {
        fprintf(stderr, "Unable to open audio: %s\n", SDL_GetError());
        return false;
    }

    //Mix_AllocateChannels(4);

    return true;
}

void SdlAudio::playSample(const std::string& name) const
{
    SdlSamplePtr sample = SdlSample::loadSample(m_resources, name);

    if (sample)
        Mix_PlayChannel(-1, sample->m_sample, 0);
}
