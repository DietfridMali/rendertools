#pragma once

#include "glew.h"

#include "array.hpp"
#include "array.hpp"
#include "viewport.h"
#include "texture.h"
#include "colordata.h"

// =================================================================================================

#define INVALID_BUFFER_INDEX 0x7FFFFFFF

class BufferInfo {
public:
    typedef enum {
        btColor,
        btDepth,
        btVertex
    } eBufferType;


    SharedTextureHandle m_handle;
    int                 m_attachment;
    int                 m_tmuIndex;
    eBufferType         m_type;

    BufferInfo(GLuint handle = 0, int attachment = 0)
        : m_handle(handle), m_attachment(attachment), m_tmuIndex(-1), m_type(btColor)
    { }
};

// =================================================================================================

class FBO {
public:
    String                      m_name;
    SharedFramebufferHandle     m_handle;
    int                         m_width;
    int                         m_height;
    int                         m_scale;
    int                         m_bufferCount;
    int                         m_vertexBufferIndex;
    int                         m_depthBufferIndex;
    ManagedArray<BufferInfo>    m_bufferInfo;
    ManagedArray<GLuint>        m_drawBuffers;
    Viewport                    m_viewport;
    Viewport*                   m_viewportSave;
    bool                        m_pingPong;
    bool                        m_isAvailable;
    bool                        m_isEnabled;
    int                         m_lastBufferIndex;
    int                         m_lastDestination;
    BaseQuad                    m_viewportArea;

    struct FBOBufferParams {
        String name = "";
        int colorBufferCount = 1;
        int depthBufferCount = 0;
        int vertexBufferCount = 0;
        bool hasMRTs = false;
    };

    struct FBORenderParams {
        int source = 0;
        int destination = -1;
        bool clearBuffer = true;
        int flipVertically = 0; // -1: flip, 1: don't flip, 0: renderer decides
        float scale = 1.0f;
        Shader* shader = nullptr;
    };

    FBO();

    ~FBO() {
        //Destroy();
    }

    bool Create(int width, int height, int scale, const FBOBufferParams& params);

    void Destroy(void);

    bool Enable(int bufferIndex = 0, bool clearBuffer = false, bool reenable = false);

    bool EnableBuffer(int bufferIndex, bool clearBuffer, bool reenable);

    void Disable(void);

    Viewport SetViewport(void);

    void RestoreViewport(void);

    inline void Reenable(void) {
        if (m_lastBufferIndex != INVALID_BUFFER_INDEX)
            Enable(m_lastBufferIndex, false, true);
    }

    void Fill(RGBAColor color);

    void Clear(int bufferIndex, bool clearBuffer);

    bool RenderTexture(Texture* texture, const FBORenderParams& params, const RGBAColor& color);

    inline bool RenderTexture(Texture* texture, const FBORenderParams& params, RGBAColor&& color) {
        return RenderTexture(texture, params, static_cast<const RGBAColor&>(color));
    }

    inline bool RenderTexture(Texture* texture, const FBORenderParams& params) {
        return RenderTexture(texture, params, ColorData::White);
    }

    bool Render(const FBORenderParams& params, const RGBAColor& color);

    inline bool Render(const FBORenderParams& params, RGBAColor&& color) {
        return Render(params, static_cast<const RGBAColor&>(color));
    }

    inline bool Render(const FBORenderParams& params) {
        return Render(params, ColorData::White);
    }

    bool AutoRender(const FBORenderParams& params, const RGBAColor& color);

    bool AutoRender(const FBORenderParams& params, RGBAColor&& color) {
        return AutoRender(params, static_cast<const RGBAColor&>(color));
    }

    bool AutoRender(const FBORenderParams& params) {
        return AutoRender(params, ColorData::White);
    }

    bool RenderToScreen(const FBORenderParams& params, const RGBAColor&);

    bool RenderToScreen(const FBORenderParams& params, RGBAColor&& color) {
        return RenderToScreen(params, static_cast<const RGBAColor&>(color));
    }

    inline bool RenderToScreen(const FBORenderParams& params) {
        return RenderToScreen(params, ColorData::White);
    }

    inline bool IsAvailable(void) {
        return m_isAvailable;
    }

    inline bool IsEnabled(void) {
        return m_isEnabled;
    }

    inline int GetLastDestination(void) {
        return m_lastDestination;
    }

    inline void SetLastDestination(int i) {
        m_lastDestination = i;
    }

    inline int NextBuffer(int i) {
        return (i + 1) % m_bufferCount;
    }

    inline GLuint GetHandle(int bufferIndex) {
        return m_bufferInfo[bufferIndex].m_handle;
    }

    bool AttachBuffer(int bufferIndex);

    bool DetachBuffer(int bufferIndex);

    bool BindBuffer(int bufferIndex, int tmuIndex = -1);

    void ReleaseBuffers(void);

    SharedTextureHandle& BufferHandle(int bufferIndex) {
#ifdef _DEBUG
        if (bufferIndex < m_bufferCount)
            return m_bufferInfo[bufferIndex].m_handle;
        return Texture::nullHandle;
#else
        return (bufferIndex < m_bufferCount) ? m_bufferInfo[bufferIndex].m_handle : Texture::nullHandle;
#endif
    }

    inline int VertexBufferIndex(int i = 0) {
        return m_vertexBufferIndex + i;
    }


    inline int DepthBufferIndex(void) {
        return m_depthBufferIndex;
    }


    inline bool operator==(const FBO& other) const {
        return m_handle == other.m_handle;
    }


    inline bool operator!=(const FBO& other) const {
        return m_handle != other.m_handle;
    }



private:
    void CreateBuffer(int bufferIndex, int& attachmentIndex, BufferInfo::eBufferType bufferType, bool isMRT);

    int CreateSpecialBuffers(BufferInfo::eBufferType bufferType, int& attachmentIndex, int bufferCount);

    bool AttachBuffers(bool hasMRTs);

    void CreateRenderArea(void);

    void SelectDrawBuffer(int bufferIndex = 0, bool reenable = false);
};

// =================================================================================================
