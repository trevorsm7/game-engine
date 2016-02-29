#ifndef __SDLTEXTURE_H__
#define __SDLTEXTURE_H__

#include "IResource.h"
#include "ResourceManager.h"

#include <SDL2/SDL.h>

class SdlTexture;
typedef std::shared_ptr<SdlTexture> SdlTexturePtr;

class SdlTexture : public IResource
{
    SDL_Texture* m_texture;

public:
    SdlTexture(SDL_Texture* texture): m_texture(texture) {}
    ~SdlTexture() override;

    SDL_Texture* getPtr() {return m_texture;}

    static SdlTexturePtr loadTexture(ResourceManager& manager, SDL_Renderer* renderer, std::string filename);

protected:
    static SDL_Texture* createTexture(SDL_Renderer* renderer, int width, int height, const void* data, int pitch, uint32_t format);

private:
    static SdlTexturePtr loadTGA(SDL_Renderer* renderer, std::vector<char>& data);

    static SdlTexturePtr getPlaceholder(SDL_Renderer* renderer);
    static SdlTexturePtr m_placeholder;
};

#endif
