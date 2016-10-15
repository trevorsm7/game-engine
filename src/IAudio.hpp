#pragma once

#include <string>

class IAudio
{
public:
    virtual ~IAudio() {}

    virtual void playSample(const std::string& name) const = 0;
};
