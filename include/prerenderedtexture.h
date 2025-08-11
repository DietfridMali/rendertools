#pragma once


#include "glew.h"
#include "vector.hpp"
#include "colordata.h"
#include "rectangle.h"
#include "base_quad.h"
#include "texture.h"
#include "viewport.h"
#include "fbo.h"
#include "textrenderer.h"

// =================================================================================================

class PrerenderedItem {
public:
    FBO         m_fbo;
    Viewport    m_viewport;
    int         m_bufferCount;

    PrerenderedItem()
        : m_bufferCount(0)
    { }

    PrerenderedItem(Viewport& viewport)
        : m_bufferCount(0), m_viewport(viewport)
    { }

    bool Create(int bufferCount = 1);

    void Destroy() {
        m_fbo.Destroy();
    }

    virtual void Render(void) {}
};

// =================================================================================================

class PrerenderedText 
    : public PrerenderedItem 
    , public TextRenderer
{
public:
    String      m_text;
#if 0
    RGBAColor   m_color;
    float       m_outlineWidth;
    Vector4f    m_outlineColor;
    float       m_scale;
#endif

    PrerenderedText()
        : PrerenderedItem()
        , TextRenderer()
        , /*m_color(ColorData::White), m_outlineWidth(0.0f), m_outlineColor(ColorData::Invisible), m_scale(1.0f),*/ m_text("")
    { }

    PrerenderedText(Viewport viewport, RGBAColor color = ColorData::White, const TextDecoration& decoration = {}, float scale = 1.0f)
        : PrerenderedItem(viewport)
        , TextRenderer(color, decoration, scale)
        , m_text("")
    { }

    void Destroy() {
        m_text.Destroy();
        PrerenderedItem::Destroy();
    }

    bool Create(String text, bool centered, int bufferCount = 0, const TextDecoration& decoration = {});

    inline void SetColor(RGBAColor color) {
        m_color = color;
    }

    inline void SetAlpha(float alpha) {
        m_color.A() = alpha;
    }

    void SetScale(float scale = 1.0) {
        m_scale = scale;
    }

    inline void SetOutlineWidth(float outlineWidth = 0) {
        m_decoration.outlineWidth = outlineWidth;
    }

    void RenderOutline(const TextDecoration& decoration = {});

    virtual void Render(bool setViewport = true, bool flipVertically = false, RGBAColor color = ColorData::Invisible, float scale = 0.0f);
};

// =================================================================================================

class PrerenderedImage : public PrerenderedItem {
public:
    Texture     m_image;
    RGBAColor   m_backgroundColor;
    float       m_scale;
    BaseQuad   m_viewportArea;

    PrerenderedImage(Texture& image, Viewport viewport, RGBAColor backgroundColor = ColorData::White, float scale = 1.0f)
        : PrerenderedItem(viewport), m_image(image), m_backgroundColor(backgroundColor), m_scale(scale)
    {
    }

    void Create(void);

    virtual void Render(void);
};

// =================================================================================================


