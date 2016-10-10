#pragma once

#include "IResource.hpp"
#include "ResourceManager.hpp"

struct Mix_Chunk;

class SdlSample;
typedef std::shared_ptr<SdlSample> SdlSamplePtr;

class SdlSample : public IResource
{
    Mix_Chunk* m_sample;

protected:
    SdlSample(Mix_Chunk* sample): m_sample(sample) {}

public:
    ~SdlSample() override;

    Mix_Chunk* getPtr() {return m_sample;}

    void playSample();

    static SdlSamplePtr loadSample(ResourceManager& manager, const std::string& filename);
};
