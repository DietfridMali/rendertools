#pragma once

#include "vector.hpp"
#include "texcoord.h"
#include "shader.h"
#include "vao.h"
#include "vertexdatabuffers.h"
#include "texturehandler.h"
#include "colordata.h"
#include "plane.h"

// =================================================================================================

class Quad 
    : public Plane
{
    public:
        static VAO*     m_vao;

        VertexBuffer    m_vertexBuffer;
        TexCoordBuffer  m_texCoordBuffer;
        VertexBuffer    m_normalBuffer;
        TexCoord        m_maxTexCoord;
        Texture*        m_texture;
        RGBAColor       m_color;
        float           m_borderWidth; // percentage of texture height
        float           m_aspectRatio;
        float           m_maxDistance;
        float           m_offset;
        bool            m_isAvailable;

    Quad() 
        : m_texture (nullptr), m_color(RGBAColor(1,1,1,1)), m_borderWidth(0), m_aspectRatio(1), m_maxDistance(0), m_offset(0), m_isAvailable(false)
    { }


    Quad(std::initializer_list<Vector3f> vertices, std::initializer_list<TexCoord> texCoords = {}, Texture* texture = nullptr, RGBAColor color = ColorData::White/*, float borderWidth = 0.0f*/)
        : Plane(vertices), m_texture(texture), m_color(color), /*m_borderWidth(borderWidth),*/ m_isAvailable(true), m_offset(0)
    {
        Setup(vertices, texCoords, texture, color/*, borderWidth*/);
    }


    Quad(const Quad& other) {
		Copy(other);
    }


    bool Setup(std::initializer_list<Vector3f> vertices, std::initializer_list<TexCoord> texCoords = {}, Texture* texture = nullptr, RGBAColor color = ColorData::White/*, float borderWidth = 0.0f*/);


    bool CreateVAO(void);


    Quad& Copy(const Quad& other);


    Quad& Move(Quad& other);


    Quad& operator= (const Quad& other) {
		return Copy(other);
    }


    Quad operator= (Quad&& other) noexcept {
        return Move(other);
    }


    void Destroy(void);

    void CreateTexCoords(void);

    void CreateNormals(void);

    bool UpdateVAO(void);

    float ComputeAspectRatio(void);

    inline void SetTexture(Texture* texture) {
        m_texture = texture;
    }

    void SetColor(RGBAColor color) {
        m_color = color;
    }

    Shader* LoadShader(bool useTexture, const RGBAColor& color = ColorData::White, float maxDistance = 0, float offset = 0);

    inline Shader* LoadShader(bool useTexture, RGBAColor&& color, float maxDistance, float offset) {
        return LoadShader(useTexture, static_cast<const RGBAColor&>(color), maxDistance, offset);
    }

    void UpdateShader(Shader* shader);

    inline void SetMaxDistance(float maxDistance) {
		m_maxDistance = maxDistance;
	}

    void Render(RGBAColor color = ColorData::White, float maxDistance = 0.0f, float offset = 0.0f);

    void Render(Shader* shader, Texture* texture, bool updateVAO = true);

    void Render(Texture* texture);

    void Fill(RGBColor color, float alpha = 1.0, float offset = 0.0);

    inline void Fill(RGBAColor color, float offset = 0.0) {
        return Fill(static_cast<RGBColor>(color), color.A(), offset);
    }

    ~Quad() {
        Destroy();
    }
};

// =================================================================================================
