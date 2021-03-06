#include <Rendering/Texture.h>
#include <Rendering/Surface.h>
#include <Rendering/Renderer.h>

#include <System/Logger.h>

using namespace Sourcehold::System;
using namespace Sourcehold::Rendering;

Texture::Texture(std::shared_ptr<Renderer> rend) :
    Renderable(),
    angle(0.0),
    width(0),
    height(0),
    flip(SDL_FLIP_NONE),
    renderer(rend)
{
}

Texture::Texture(const Texture &tex) :
    Renderable(),
    angle(0.0),
    width(0),
    height(0),
    flip(SDL_FLIP_NONE)
{
    this->renderer = tex.renderer;
    this->texture = tex.texture;
    this->width = tex.width;
    this->height = tex.height;
}

Texture::~Texture() {
    if(valid) SDL_DestroyTexture(texture);
}

bool Texture::AllocNewStreaming(int width, int height, int format) {
    this->width = width;
    this->height = height;
    texture = SDL_CreateTexture(
        renderer->GetRenderer(),
        format,
        SDL_TEXTUREACCESS_STREAMING,
        width, height
    );
    if(!texture) {
        Logger::error("RENDERING") << "Unable to create texture: " << SDL_GetError() << std::endl;
        return false;
    }

    /* Enable transparency */
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    valid = true;
    return true;
}

bool Texture::AllocFromSurface(Surface &surface) {
    texture = SDL_CreateTextureFromSurface(renderer->GetRenderer(), surface.GetSurface());
    if(!texture) {
        Logger::error("RENDERING") << "Unable to create texture from surface: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    valid = true;
    return true;
}

void Texture::UpdateTexture() {

}

void Texture::Destroy() {

}

void Texture::LockTexture() {
    if(locked) return;
    if(SDL_LockTexture(texture, nullptr, (void**)&pixels, &pitch)) {
        Logger::error("RENDERING") << "Unable to lock texture: " << SDL_GetError() << std::endl;
        locked = false;
    }else locked = true;
}

void Texture::UnlockTexture() {
    if(!locked) return;
    SDL_UnlockTexture(texture);
    locked = false;
}

void Texture::SetPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if(!locked) return;
    uint32_t index = x + y * width;
    pixels[index] = ToPixel(r, g, b, a);
}

void Texture::Rotate(double angle) {
    this->angle += angle;
}

void Texture::FlipHorizontal() {
    flip = (SDL_RendererFlip)((int)flip | SDL_FLIP_HORIZONTAL);
}

void Texture::FlipVertical() {
    flip = (SDL_RendererFlip)((int)flip | SDL_FLIP_VERTICAL);
}

void Texture::FlipNone() {
    flip = (SDL_RendererFlip)SDL_FLIP_NONE;
}

void Texture::SetAlphaMod(Uint8 alpha) {
    SDL_SetTextureAlphaMod(texture, alpha);
}

void Texture::Copy(Texture &other, uint32_t x, uint32_t y, SDL_Rect *rect) {
    if(!locked || !other.IsLocked()) {
        Logger::error("RENDERING") << "Lock the texture before copying from or to it!" << std::endl;
        return;
    }

    uint32_t offX = 0, offY = 0, ow = other.GetWidth(), oh = other.GetHeight();
    if(rect) {
        offX = rect->x;
        offY = rect->y;
        ow = rect->w;
        oh = rect->h;
    }

    if( x + ow > width || y + oh > height ) {
        Logger::error("RENDERING") << "Attempted to copy a texture which is too large for the target (or goes out of bounds)!" << std::endl;
        return;
    }

    for(uint32_t ix = 0; ix < ow; ix++) {
        for(uint32_t iy = 0; iy < oh; iy++) {
            uint32_t index1 = (ix+offX) + (iy+offY) * other.GetWidth();
            uint32_t index2 = (x+ix) + (y+iy) * width;

            Uint8 r, g, b, a;
            FromPixel(other.GetData()[index1], &r, &g, &b, &a);

            if(a != 0) pixels[index2] = ToPixel(r, g, b, a);
        }
    }
}

uint32_t *Texture::GetData() {
    if(!locked) return nullptr;
    return pixels;
}

Uint32 Texture::ToPixel(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
   return r << 24 | g << 16 | b << 8 | a;
}

void Texture::FromPixel(Uint32 value, Uint8 *r, Uint8 *g, Uint8 *b, Uint8 *a) {
    *r = value >> 24;
    *g = (value >> 16) & 0b11111111;
    *b = (value >> 8) & 0b1111111111111111;
    *a = value & 0b111111111111111111111111;
}
