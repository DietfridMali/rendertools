#include <utility>
#include <stdio.h>
#include "texture.h"
#include "SDL_image.h"
#include "appdata.h"

// =================================================================================================

SharedTextureHandle Texture::nullHandle = SharedTextureHandle(0);

TextureBuffer::TextureBuffer(SDL_Surface* source, bool flipVertically) {
    Create(source, flipVertically);
}


TextureBuffer::TextureBuffer(TextureBuffer const& other) {
    Copy(const_cast<TextureBuffer&> (other));
}


TextureBuffer::TextureBuffer(TextureBuffer&& other) noexcept {
    Move(other);
}


void TextureBuffer::Reset(void) {
    m_info.Reset();
#if USE_SHARED_POINTERS
    m_data.Release();
#else
    m_data = nullptr;
#endif
}


void TextureBuffer::FlipSurface(SDL_Surface* source)
{
    SDL_LockSurface(source);
    int pitch = source->pitch; // row size
    int h = source->h;
    char* pixels = (char*)source->pixels + h * pitch;
    char* dataPtr = (char*)m_data;

    for (int i = 0; i < h; i++) {
        pixels -= pitch;
        memcpy(dataPtr, pixels, pitch);
        dataPtr += pitch;
    }
    SDL_UnlockSurface(source);
}


TextureBuffer& TextureBuffer::Create(SDL_Surface* source, bool flipVertically) {
    m_info.width = source->w;
    m_info.height = source->h;
    m_info.componentCount = source->pitch / source->w;
    if (m_info.componentCount < 3) {
        SDL_Surface* h = source;
        source = SDL_ConvertSurfaceFormat(source, SDL_PIXELFORMAT_RGBA32, 0);
        SDL_FreeSurface(h);
        m_info.componentCount = source->pitch / source->w;
    }
    m_info.internalFormat = (m_info.componentCount == 4) ? GL_RGBA : GL_RGB;
    m_info.format = (m_info.componentCount == 4) ? GL_RGBA : GL_RGB;
    m_info.dataSize = m_info.width * m_info.height * m_info.componentCount;
#if USE_SHARED_POINTERS
    m_data = SharedPointer<char>(m_info.dataSize);
#else
    m_data = new char[m_info.dataSize];
#endif
    if (not m_data) // malloc(m_dataSize);
        fprintf(stderr, "%s (%d): memory allocation for texture clone failed\n", __FILE__, __LINE__);
    else {
        if (flipVertically)
            FlipSurface(source);
        else
            memcpy(m_data, source->pixels, m_info.dataSize);
        SDL_FreeSurface(source);
    }
    return *this;
}


TextureBuffer& TextureBuffer::operator= (const TextureBuffer& other) {
    return Copy(const_cast<TextureBuffer&>(other));
}

TextureBuffer& TextureBuffer::operator= (TextureBuffer&& other) {
    return Move(other);
}

TextureBuffer& TextureBuffer::Copy(TextureBuffer& other) {
    m_info = other.m_info;
    m_data = other.m_data;
    return *this;
}

TextureBuffer& TextureBuffer::Move(TextureBuffer& other) {
    m_info = other.m_info;
    m_data = std::move(other.m_data);
    other.Reset();
    return *this;
}

// =================================================================================================

bool Texture::Create(void) {
    Destroy();
#if USE_SHARED_HANDLES
    m_handle = SharedTextureHandle();
    return m_handle.Claim() != 0;
#else
    glGenTextures(1, &m_handle);
    return m_handle != 0;
#endif
}


void Texture::Destroy(void) {
#if USE_SHARED_HANDLES
    m_handle.Release();
#else
    glDeleteTextures(1, &m_handle);
    m_handle = 0;
#endif
    TextureBuffer* texBuf = nullptr;
    for (const auto& p : m_buffers) {
        if (p != texBuf) {
            texBuf = p;
            delete p;
        }
    }
    m_buffers.Clear();
}


Texture& Texture::Copy(const Texture& other) {
    if (this != &other) {
        Destroy();
        m_handle = other.m_handle;
        m_name = other.m_name;
        m_buffers = other.m_buffers;
        m_filenames = other.m_filenames;
        m_type = other.m_type;
        m_wrapMode = other.m_wrapMode;
        m_useMipMaps = other.m_useMipMaps;
    }
    return *this;
}


Texture& Texture::Move(Texture& other) {
    if (this != &other) {
        Destroy();
        m_handle = std::move(other.m_handle);
#if !USE_SHARED_HANDLES
        other.m_handle = 0;
#endif
        m_name = std::move(other.m_name);
        m_buffers = std::move(other.m_buffers);
        m_filenames = std::move(other.m_filenames);
        m_type = other.m_type;
        m_wrapMode = other.m_wrapMode;
        m_useMipMaps = other.m_useMipMaps;
    }
    return *this;
}

bool Texture::IsAvailable(void) {
    return 
#if USE_SHARED_HANDLES
        m_handle.IsAvailable() 
#else
        (m_handle != 0)
#endif
        && (HasBuffer() or not m_buffers.IsEmpty());
}


void Texture::Bind(void) {
    if (IsAvailable()) {
        glEnable(GL_TEXTURE_2D);
        glActiveTexture(GL_TEXTURE0);
#if USE_SHARED_HANDLES
        glBindTexture(m_type, m_handle.get());
#else
        glBindTexture(m_type, m_handle);
#endif
    }
}


void Texture::Release(void) {
    if (IsAvailable())
        glBindTexture(m_type, 0);
}


void Texture::SetParams(void) {
    if (m_useMipMaps) {
        glTexParameteri(m_type, GL_GENERATE_MIPMAP, GL_TRUE);
        glTexParameteri(m_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(m_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else {
        glTexParameteri(m_type, GL_GENERATE_MIPMAP, GL_FALSE);
        glTexParameteri(m_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(m_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}


void Texture::Wrap(void) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_wrapMode);
}


void Texture::Enable(int tmu) {
    glActiveTexture(GL_TEXTURE0 + tmu);
    glEnable(m_type);
    Bind();
    SetParams();
    Wrap();
}


void Texture::Disable(void) {
    Release();
    glDisable(m_type);
}


void Texture::Deploy(int bufferIndex) {
    if (IsAvailable()) {
        Bind();
        SetParams();
        TextureBuffer* texBuf = m_buffers[bufferIndex];
        glTexImage2D(m_type, 0, texBuf->m_info.internalFormat, texBuf->m_info.width, texBuf->m_info.height, 0, texBuf->m_info.format, GL_UNSIGNED_BYTE, texBuf->m_data);
        Release();
    }
}

// --------------------------------------------------------------------------------
// Load loads textures from file. The texture filenames are given in filenames
// An empty filename ("") means that the previously loaded texture should be used here as well
// This makes sense e.g. for cubemaps if several of its faces share the same texture, like e.g. spherical smileys,
// which have a face on one side and flat color on the remaining five sides of the cubemap used to texture them.
// So a smiley cubemap texture list could be specified here like this: ("skin.png", "", "", "", "", "face.png")
// This would cause the skin texture to be reused for in the place of the texture data buffers at positions 2, 3, 4
// and 5. You could also do something like ("skin.png", "", "back.png", "", "face.png") or just ("color.png", "", "", "", "", "")
// for a uniformly textured sphere. The latter case will however be also taken into regard by the cubemap class.
// It allows to pass a single texture which it will use for all faces of the cubemap

bool Texture::Load(List<String>& fileNames, bool flipVertically) {
    // load texture from file
    m_filenames = fileNames;
    m_name = fileNames.First();
    TextureBuffer* texBuf = nullptr;
#ifdef _DEBUG
    String bufferName = "";
#endif
    for (auto& fileName : fileNames) {
        if (fileName.IsEmpty()) { // must never be true for first filename
            if (not texBuf) // must always be true here -> fileNames[0] must always contain a valid filename of an existing texture file
                throw std::runtime_error("Texture::Load: missing texture names");
            else {
                m_buffers.Append(texBuf);
#ifdef _DEBUG
                texBuf->m_name = bufferName;
#endif
            }
        }
        else {
#ifdef _DEBUG
            bufferName = fileName;
#endif
            SDL_Surface* image = IMG_Load(fileName.Data());
            if (not image) {
                fprintf(stderr, "Couldn't find '%s'\n", (char*)(fileName));
                return false;
            }
            texBuf = new TextureBuffer();
            if (texBuf) {
                texBuf->Create(image, flipVertically);
                m_buffers.Append(texBuf);
#ifdef _DEBUG
                texBuf->m_name = bufferName;
#endif
            }
        }
    }
    return true;
}


bool Texture::CreateFromFile(List<String>& fileNames, bool flipVertically) {
    if (not Create())
        return false;
    if (fileNames.IsEmpty())
        return true;
    if (not Load(fileNames, flipVertically))
        return false;
    Deploy();
    return true;
}


bool Texture::CreateFromSurface(SDL_Surface* surface) {
    if (not Create())
        return false;
    m_buffers.Append(new TextureBuffer(surface, appData->flipImagesVertically));
    return true;
}


tRenderOffsets Texture::ComputeOffsets(int w, int h, int viewportWidth, int viewportHeight, int renderAreaWidth, int renderAreaHeight)
{
    if (renderAreaWidth == 0)
        renderAreaWidth = viewportWidth;
    if (renderAreaHeight == 0)
        renderAreaHeight = viewportHeight;
    float xScale = float(renderAreaWidth) / float(viewportWidth);
    float yScale = float(renderAreaHeight) / float(viewportHeight);
    float wRatio = float(renderAreaWidth) / float(w);
    float hRatio = float(renderAreaHeight) / float(h);
    tRenderOffsets offsets = { 0.5f * xScale, 0.5f * yScale };
    if (wRatio > hRatio)
        offsets.x -= (float(renderAreaWidth) - float(w) * hRatio) / float(2 * viewportWidth);
    else if (wRatio < hRatio)
        offsets.y -= (float(renderAreaHeight) - float(h) * wRatio) / float(2 * viewportHeight);
    return offsets;
}

// =================================================================================================
