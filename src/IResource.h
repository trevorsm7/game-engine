#ifndef __IRESOURCE_H__
#define __IRESOURCE_H__

#include <memory>

class IResource
{
public:
    virtual ~IResource() {}

    // TODO: any common functions here?
    // TODO: drop in favor of ITexture, IMesh, etc?
};

typedef std::shared_ptr<IResource> IResourcePtr;

#endif
