#pragma once

#include "SDL_ttf.h"

#include "vector.hpp"
#include "base_quad.h"
#include "fbo.h"
#include "dictionary.hpp"
#include "outlinerenderer.h"

// =================================================================================================

class TextRenderer : public OutlineRenderer {
public:
    TTF_Font*       m_font;
    String          m_euroChar;
    String          m_characters;
    bool            m_isAvailable;
    RGBAColor       m_color;
    RGBAColor       m_outlineColor;
    float           m_scale;
    bool            m_centerText;
    OutlineRenderer::tAAMethod m_aaMethod;

    typedef struct {
        int width;
        int height;
        float aspectRatio;
    } tTextDimensions;

    Dictionary<String, Texture*> m_textures;
    Dictionary<int, FBO*>        m_fbos;

    static int CompareFBOs(void* context, const int& key1, const int& key2);

    static int CompareTextures(void* context, const char& key1, const char& key2);

    TextRenderer();

    bool Create(String fontFolder, String fontName);

    void Fill(Vector4f color);

    void RenderToFBO(String text, FBO* fbo, Viewport& viewport, int renderAreaWidth = 0, int renderAreaHeight = 0, float outlineWidth = 0., Vector4f outlineColor = Vector4f{0, 0, 0, 0});

    void RenderToScreen(FBO* fbo);

    void Render(String text, bool centered = false, int renderAreaWidth = 0, int renderAreaHeight = 0, float outlineWidth = 0., Vector4f outlineColor = Vector4f{0, 0, 0, 0});

    inline void SetColor(Vector4f color = Vector4f{1, 1, 1, 1}) {
        m_color = color;
    }

    inline void SetAlpha(float alpha = 1.0) {
        m_color.A() = alpha;
    }

    inline void SetScale(float scale = 1.0) {
        m_scale = scale;
    }

    inline Texture* FindTexture(String key) {
        Texture** texPtr = m_textures.Find(key);
        return texPtr ? *texPtr : nullptr;
    }

    inline void CenterText(bool centerText) {
        m_centerText = centerText;
    }

    void SetAAMethod(OutlineRenderer::tAAMethod aaMethod) {
        m_aaMethod = aaMethod;
    }

    tTextDimensions TextSize(String text);

private:
    bool InitFont(String fontFolder, String fontName);

    void CreateTexture(const char* szChar, char key);

    void CreateTextures(void);

    BaseQuad& CreateQuad(BaseQuad& q, float x, float y, float d, Texture* t);

    FBO* GetFBO(float scale);

    Shader* LoadShader(void);

    void RenderText(String& text, int textWidth, float xOffset, float yOffset);

    int SourceBuffer(bool hasOutline, bool antiAliased);

    static inline int FBOID(const int width, const int height) {
        return width << 16 | height;
    }

    static inline int FBOID(const FBO* fbo) {
        return FBOID (fbo->m_width, fbo->m_height);
    }

};

extern TextRenderer* textRenderer;

// =================================================================================================


