
#include "glew.h"
#include "vector.hpp"
#include "quad.h"
#include "texture.h"
#include "viewport.h"
#include "fbo.h"
#include "textrenderer.h"
#include "renderer.h"
#include "prerenderedtexture.h"

// =================================================================================================

bool PrerenderedItem::Create(int bufferCount) {
    if (m_fbo.IsAvailable() and (m_bufferCount == bufferCount)) {
        m_fbo.SetLastDestination(0);
        return false;
    }
    m_bufferCount = bufferCount;
    m_fbo.Create(m_viewport.m_width, m_viewport.m_height, 2, { .colorBufferCount = bufferCount } );
    return true;
}

// =================================================================================================

bool PrerenderedText::Create(String text, int bufferCount, OutlineRenderer::tAAMethod aaMethod) {
    if (bufferCount == 0)
        bufferCount = 2;//  (m_outlineWidth == 0) ? 1 : 2;
    if (not PrerenderedItem::Create(bufferCount) && (m_text == text))
        return false;
    m_text = text;
    textRenderer->SetColor(m_color);
    textRenderer->SetAlpha(1.0f);
    textRenderer->SetScale(1.0f);
    textRenderer->SetAAMethod(aaMethod);
    textRenderer->RenderToFBO(m_text, &m_fbo, m_fbo.m_viewport, 0, 0, m_outlineWidth, m_outlineColor); // m_outlineWidth == 0);
    textRenderer->SetColor();
    return true;
}


void PrerenderedText::RenderOutline(float outlineWidth, RGBAColor outlineColor, OutlineRenderer::tAAMethod aaMethod) {
    m_outlineWidth = outlineWidth;
    if (outlineWidth > 0) {
        m_fbo.SetViewport();
        m_fbo.SetLastDestination(0);
        textRenderer->SetAAMethod(aaMethod);
        textRenderer->RenderOutline(&m_fbo, outlineWidth * 2, outlineColor);
        m_fbo.RestoreViewport();
    }
}


void PrerenderedText::Render(bool setViewport, RGBAColor color, float alpha, float scale) {
    if (color.A() >= 0)
        textRenderer->SetColor(color);
    if (alpha >= 0)
        textRenderer->SetAlpha(alpha); // must be called after SetColor() to affect rendered text's alpha
    if (scale >= 0)
        textRenderer->SetScale(scale);
    if (setViewport)
        m_viewport.SetViewport();
    textRenderer->RenderToScreen(&m_fbo); // m_outlineWidth == 0);
    if (alpha >= 0)
        textRenderer->SetAlpha();
    if (scale >= 0)
        textRenderer->SetScale();
    if (color.A() >= 0)
        textRenderer->SetColor();
}

// =================================================================================================

void PrerenderedImage::Create(void) {
    PrerenderedItem::Create();
    m_viewport.SetViewport();
    m_viewport.Fill(static_cast<RGBColor>(m_backgroundColor), 1);
    m_fbo.RenderTexture(&m_image, { .destination = 0, .clearBuffer = true });
}


void PrerenderedImage::Render(void) {
    m_viewport.SetViewport();
    m_fbo.Render({ .source = 0, .destination = -1 });
}

// =================================================================================================

