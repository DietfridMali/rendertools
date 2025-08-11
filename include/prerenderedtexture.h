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

class PrerenderedText : public PrerenderedItem {
public:
    String      m_text;
    RGBAColor   m_color;
    float       m_outlineWidth;
    Vector4f    m_outlineColor;
    float       m_alpha;
    float       m_scale;

    PrerenderedText()
        : PrerenderedItem(), m_color(RGBAColor{ 1,1,1,-1 }), m_outlineWidth(0.0), m_outlineColor(ColorData::White), m_alpha(-1), m_scale(-1), m_text("")
    { }

    PrerenderedText(Viewport viewport, RGBAColor color = ColorData::White, float outlineWidth = 0, RGBAColor outlineColor = ColorData::White)
        : PrerenderedItem(viewport), m_color(color), m_outlineWidth(outlineWidth), m_outlineColor(outlineColor), m_alpha(-1), m_scale(-1), m_text("")
    { }

    void Destroy() {
        m_text.Destroy();
        PrerenderedItem::Destroy();
    }

    bool Create(String text, bool centered, int bufferCount = 0, OutlineRenderer::tAAMethod aaMethod = { "", 0 });

    inline void SetColor(RGBAColor color) {
        m_color = color;
    }

    inline void SetAlpha(float alpha) {
        m_alpha = alpha;
    }

    void SetScale(float scale = 1.0) {
        m_scale = scale;
    }

    inline void SetOutlineWidth(float outlineWidth = 0) {
        m_outlineWidth = outlineWidth;
    }

    void RenderOutline(float outlineWidth, RGBAColor outlineColor, OutlineRenderer::tAAMethod aaMethod = { "", 0 });

    virtual void Render(bool setViewport = true, bool flipVertically = false, RGBAColor color = RGBAColor(1.0f, 1.0f, 1.0f, -1.0f), float alpha = -1.0f, float scale = -1.0f);
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


