//pragma once

#include <algorithm>
#include "glew.h"
#include "base_renderer.h"
#include "base_shaderhandler.h"
#include "colordata.h"
#include "textrenderer.h"
#include "base_renderer.h"

#ifndef _WIN32
#   include <locale>
#endif

#define USE_TEXT_FBOS 1

// =================================================================================================

int TextRenderer::CompareFBOs(void* context, const int& key1, const int& key2) {
    return (key1 < key2) ? -1 : (key1 > key2) ? 1 : 0;
}


int TextRenderer::CompareTextures(void* context, const char& key1, const char& key2) {
    return (key1 < key2) ? -1 : (key1 > key2) ? 1 : 0;
}


TextRenderer::TextRenderer(RGBAColor color, const TextDecoration& decoration, float scale)
    : m_color(color), m_scale(scale), m_font(nullptr), m_centerText(true), m_decoration(decoration), m_isAvailable(false)
{
    Setup();
}


void TextRenderer::Setup(void) {
#ifdef _WIN32
    m_euroChar = "\xE2\x82\xAC"; // "\u20AC";
#else
    //std::locale::global(std::locale("de_DE.UTF-8"));
    m_euroChar = "\u20AC";
#endif
    m_characters = String("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz+-=.,*/: _?!%"); // +m_euroChar;
#if !(USE_STD || USE_STD_MAP)
    m_fbos.SetComparator(TextRenderer::CompareFBOs);
    m_textures.SetComparator(String::Compare); //TextRenderer::CompareTextures);
#endif
}


bool TextRenderer::InitFont(String fontFolder, String fontName) {
    if (0 > TTF_Init()) {
        fprintf(stderr, "Cannot initialize font system\n");
        return false;
    }
    String fontFile = fontFolder + fontName;
    if (not (m_font = TTF_OpenFont(fontFile.Data(), 120))) {
        fprintf(stderr, "Cannot load font '%s'\n", (char*) fontName);
        return false;
    }
    return true;
}


bool TextRenderer::Create(String fontFolder, String fontName) {
    if (m_isAvailable = InitFont(fontFolder, fontName))
        CreateTextures();
    return m_isAvailable;
}


void TextRenderer::CreateTexture(const char* szChar, char key)
{
    Texture* texture = new Texture();
    SDL_Surface* surface = (strlen(szChar) == 1)
                           ? TTF_RenderText_Solid(m_font, szChar, SDL_Color(255, 255, 255, 255))
                           : TTF_RenderUTF8_Solid(m_font, szChar, SDL_Color(255, 255, 255, 255));
    if (texture->CreateFromSurface(surface)) {
        texture->Deploy();
        m_textures.Insert(String(key), texture);
    }
}


void TextRenderer::CreateTextures(void) {
    char szChar[4] = " ";
    for (char* p = m_characters.Data(); *p; p++) {
        szChar[0] = *p;
        CreateTexture(szChar, *p);
    }
    //CreateTexture((const char*) u8"€", '€');
    CreateTexture((const char*)m_euroChar, '€');
}


BaseQuad& TextRenderer::CreateQuad(BaseQuad& q, float x, float y, float w, Texture* t) {
#if USE_TEXT_FBOS
    q.Setup({ Vector3f{x, y, 0.0}, Vector3f{x + w, y, 0.0}, Vector3f{x + w, -y, 0.0}, Vector3f{x, -y, 0.0} },
            { TexCoord{0, 1}, TexCoord{1, 1}, TexCoord{1, 0}, TexCoord{0, 0} },
            t, m_color);
#else
    x += 0.5f;
    q.Setup({ Vector3f{x, y + 0.5f, 0.0}, Vector3f{x + w, y + 0.5f, 0.0}, Vector3f{x + w, -y + 0.5f, 0.0}, Vector3f{x, -y + 0.5f, 0.0} },
            { TexCoord{0, 0}, TexCoord{1, 0}, TexCoord{1, 1}, TexCoord{0, 1} },
            t, m_color);
#endif
    return q;
}


struct TextRenderer::TextDimensions TextRenderer::TextSize(String text) {
    int w = 0;
    int h = 0;
    for (char* p = text.Data(); *p; p++) {
        Texture* t = FindTexture(String(*p));
        if (not t) {
            fprintf(stderr, "texture '%c' not found\r\n", *p);
            return { .width = 0, .height = 0, .aspectRatio = 1.0f };
        }
        int tw = t->GetWidth();
        w += tw;
        //auto Max = [=](auto& a, auto b) { return (a > b) ? a : b; };
        int th = t->GetHeight();
        if (h < th)
            h = th;
    }
    return { .width = w, .height = h, .aspectRatio = float (w) / float (h) };
}


FBO* TextRenderer::GetFBO(float scale) {
    FBO** fboRef = m_fbos.Find(FBOID(baseRenderer.Viewport().m_width, baseRenderer.Viewport().m_height));
    if (fboRef != nullptr)
        return *fboRef;
    FBO* fbo = new FBO();
    fbo->Create(baseRenderer.Viewport().m_width, baseRenderer.Viewport().m_height, 2, {.name = "text", .colorBufferCount = 2});
    m_fbos.Insert(FBOID(fbo), fbo);
    return fbo;
}


Shader* TextRenderer::LoadShader(void) {
    static ShaderLocationTable locations;
    Shader* shader = baseShaderHandler.SetupShader("plainTexture");
    if (shader) {
        locations.Start();
        shader->SetVector4f("surfaceColor", locations.Current(), ColorData::White);
    }
    return shader;
}


void TextRenderer::RenderText(String& text, int textWidth, float xOffset, float yOffset, eTextAlignments alignment) {
    baseRenderer.PushMatrix();
#if USE_TEXT_FBOS
    baseRenderer.ResetTransformation();
    baseRenderer.Translate(0.5f, 0.5f, 0.0f);
    glDepthFunc(GL_ALWAYS);
#endif
    float x, letterScale = 2 * xOffset / float(textWidth);
    switch (alignment) {
    case taLeft:
        x = -0.5f;
        break;
    case taRight:
        x = 1.0f - xOffset * 2;
        break;
    case taCenter:
    default:
        x = -xOffset;
    }
    Shader* shader = LoadShader();
    BaseQuad q;
    int i = 0;
    for (char* p = text.Data(); *p; ++p, ++i) {
        Texture* t = FindTexture(String(*p));
        if (not t) 
            fprintf(stderr, "texture '%c' not found\r\n", *p);
        else {
            int tw = t->GetWidth();
            float w = float(tw) * letterScale;
            CreateQuad(q, x, yOffset, w, t);
#if 1
            q.Render(shader, t, true);
#else
            q.Fill((i & 1) ? ColorData::Orange : ColorData::MediumBlue);
#endif
            //q.Render(m_color, m_color.A());
            x += w;
        }
    }
    //baseShaderHandler.StopShader();
    baseRenderer.PopMatrix();
#if USE_TEXT_FBOS
    glDepthFunc(GL_LESS);
#endif
}


int TextRenderer::SourceBuffer(bool hasOutline, bool antiAliased) {
    return hasOutline ? antiAliased ? 0 : 1 : antiAliased ? 1 : 0;
}


void TextRenderer::Fill(Vector4f color) {
    FBO* fbo = GetFBO(1);
    if (fbo != nullptr)
        fbo->Fill(color);
}


void TextRenderer::RenderToFBO(String text, eTextAlignments alignment, FBO* fbo, Viewport& viewport, int renderAreaWidth, int renderAreaHeight) {
    if (m_isAvailable) {
        fbo->m_name = String::Concat ("[", text, "]");
        auto [textWidth, textHeight, aspectRatio] = TextSize(text);
        float outlineWidth = m_decoration.outlineWidth * 2;
        textWidth += int (2 * outlineWidth + 0.5f);
        textHeight += int (2 * outlineWidth + 0.5f);
        tRenderOffsets offset = 
#if 1
            Texture::ComputeOffsets(int(textHeight * aspectRatio), textHeight, viewport.m_width, viewport.m_height, renderAreaWidth, renderAreaHeight);
#else
            m_centerText
            ? Texture::ComputeOffsets(int (textHeight * aspectRatio), textHeight, viewport.m_width, viewport.m_height, renderAreaWidth, renderAreaHeight)
            : Texture::ComputeOffsets(int(textHeight * aspectRatio), textHeight, int(textHeight * aspectRatio), textHeight, int(textHeight * aspectRatio), textHeight);
#endif
        textWidth -= int (2 * outlineWidth + 0.5f);
        textHeight -= int (2 * outlineWidth + 0.5f);
#if USE_TEXT_FBOS
        if (fbo->Enable(0, true)) {
            fbo->SetViewport();
            if (outlineWidth > 0) {
                offset.x -= outlineWidth / float(fbo->m_width);
                offset.y -= outlineWidth / float(fbo->m_height);
            }
            fbo->m_lastDestination = 0;
            RenderText(text, textWidth, offset.x, offset.y, alignment);
            fbo->Disable();
            if (fbo->IsAvailable()) {
                if (HaveOutline())
                    RenderOutline(fbo, m_decoration);
                else if (ApplyAA())
                    AntiAlias(fbo, m_decoration.aaMethod);
            }
            fbo->RestoreViewport();
        }
#else
        RenderText(text, textWidth, offset.x, offset.y, taCentered);
#endif
#if USE_TEXT_FBOS
#endif
    }
}


void TextRenderer::RenderToScreen(FBO* fbo, bool flipVertically) {
#if USE_TEXT_FBOS
    if (m_isAvailable)
        fbo->RenderToScreen({ .source = fbo->GetLastDestination(), .clearBuffer = false, .flipVertically = flipVertically, .scale = m_scale }, m_color); // render outline to viewport
#endif
}


void TextRenderer::Render(String text, eTextAlignments alignment, bool flipVertically, int renderAreaWidth, int renderAreaHeight) {
    if (m_isAvailable) {
        FBO* fbo = GetFBO(2.0f);
        if (fbo != nullptr) {
            RenderToFBO(text, alignment, fbo, baseRenderer.Viewport(), renderAreaWidth, renderAreaHeight);
            RenderToScreen(fbo, flipVertically); // render outline to viewport
        }
    }
}

// =================================================================================================
