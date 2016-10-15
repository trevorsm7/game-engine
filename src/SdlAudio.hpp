#pragma once

#include "IAudio.hpp"
#include "IResource.hpp"
#include "ResourceManager.hpp"

struct Mix_Chunk;

class SdlSample;
typedef std::shared_ptr<SdlSample> SdlSamplePtr;

class SdlSample : public IResource
{
    friend class SdlAudio;

    Mix_Chunk* m_sample;

protected:
    SdlSample(Mix_Chunk* sample): m_sample(sample) {}

public:
    ~SdlSample() override;

    Mix_Chunk* getPtr() {return m_sample;}

    static SdlSamplePtr loadSample(ResourceManager& manager, const std::string& filename);
};

class SdlAudio : public IAudio
{
     ResourceManager& m_resources;

public:
    SdlAudio(ResourceManager& resources): m_resources(resources) {}
    ~SdlAudio() override;

    bool init();

    void playSample(const std::string& name) const override;
};
