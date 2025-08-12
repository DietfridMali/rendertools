#pragma once

#include "SDL_ttf.h"

#include "vector.hpp"
#include "base_quad.h"
#include "fbo.h"
#include "dictionary.hpp"
#include "colordata.h"
#include "outlinerenderer.h"
#include "singletonbase.hpp"

// =================================================================================================

class TextRenderer 
    : public OutlineRenderer 
    , public BaseSingleton<TextRenderer>
{
public:
    using TextDecoration = OutlineRenderer::Decoration;

    typedef enum {
        taLeft,
        taCenter,
        taRight
    } eTextAlignments;

    TTF_Font*               m_font;
    String                  m_euroChar;
    String                  m_characters;
    bool                    m_isAvailable;
    RGBAColor               m_color;
    float                   m_scale;
    bool                    m_centerText;
    struct TextDecoration   m_decoration;

    struct TextDimensions {
        int width = 0;
        int height = 0;
        float aspectRatio = 0.0f;
    };

    Dictionary<String, Texture*> m_textures;
    Dictionary<int, FBO*>        m_fbos;

    static int CompareFBOs(void* context, const int& key1, const int& key2);

    static int CompareTextures(void* context, const char& key1, const char& key2);

    TextRenderer(RGBAColor color = ColorData::White, const TextDecoration& decoration = {}, float scale = 1.0f);

    void Setup(void);

    bool Create(String fontFolder, String fontName);

    void Fill(Vector4f color);

    void RenderToFBO(String text, eTextAlignments alignment, FBO* fbo, Viewport& viewport, int renderAreaWidth = 0, int renderAreaHeight = 0);

    void RenderToScreen(FBO* fbo, bool flipVertically = false);

    void Render(String text, eTextAlignments alignment = taLeft, bool flipVertically = false, int renderAreaWidth = 0, int renderAreaHeight = 0);

    inline bool SetColor(RGBAColor color = ColorData::White) {
        if (color.A() < 0.0f)
            return false;
        m_color = color;
        return true;
    }

    inline bool SetAlpha(float alpha = 1.0) {
        if (alpha < 0.0f)
            return false;
        m_color.A() = alpha;
        return true;
    }

    inline bool SetScale(float scale = 1.0) {
        if (scale < 0.0f)
            return false;
        m_scale = scale;
        return true;
    }

    void SetAAMethod(const OutlineRenderer::AAMethod& aaMethod) {
        m_decoration.aaMethod = aaMethod;
    }

    inline void CenterText(bool centerText) {
        m_centerText = centerText;
    }

    inline void SetOutline(float outlineWidth = 0.0f, RGBAColor outlineColor = ColorData::Invisible) {
        m_decoration.outlineWidth = outlineWidth;
        m_decoration.outlineColor = outlineColor;
    }


    inline void SetDecoration(const TextDecoration& decoration = {}) {
        m_decoration = decoration;
    }

    inline bool HaveOutline(void) {
        return m_decoration.HaveOutline();
    }

    inline bool ApplyAA(void) {
        return m_decoration.ApplyAA();
    }

    inline Texture* FindTexture(String key) {
        Texture** texPtr = m_textures.Find(key);
        return texPtr ? *texPtr : nullptr;
    }

    struct TextDimensions TextSize(String text);

private:
    bool InitFont(String fontFolder, String fontName);

    void CreateTexture(const char* szChar, char key);

    void CreateTextures(void);

    BaseQuad& CreateQuad(BaseQuad& q, float x, float y, float w, Texture* t);

    FBO* GetFBO(float scale);

    Shader* LoadShader(void);

    void RenderText(String& text, int textWidth, float xOffset, float yOffset, eTextAlignments alignment = taLeft);

    int SourceBuffer(bool hasOutline, bool antiAliased);

    static inline int FBOID(const int width, const int height) {
        return width << 16 | height;
    }

    static inline int FBOID(const FBO* fbo) {
        return FBOID (fbo->m_width, fbo->m_height);
    }

};

#define textRenderer TextRenderer::Instance()

// =================================================================================================


