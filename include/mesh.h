#pragma once

#define NOMINMAX

#include <limits>

#include "glew.h"
#include "sharedpointer.hpp"
#include "texture.h"
#include "vao.h"
#include "vertexdatabuffers.h"

// =================================================================================================
// Mesh class definitions for basic mesh information, allowing to pass child classes to functions
// operating with or by meshes
// A mesh is defined by a set of faces, which in turn are defined by vertices, and a set of textures
// The only mesh used in Smiley Battle are ico spheres

class AbstractMesh {
public:
    virtual void Create(int quality, Texture* texture, List<String> textureNames) = 0;

    virtual void Destroy(void) = 0;

    virtual void Render(Shader* shader, Texture* texture) = 0;
};

// =================================================================================================

class MeshColors {
public:
    List<RGBAColor> m_colors;

    void Push(RGBAColor color) {
        m_colors.Append(color);
    }


    void Pop(void) {
        m_colors.Extract(-1);
    }


    RGBAColor Get(void) {
        if (not m_colors.IsEmpty())
            return m_colors[-1];
        return RGBAColor{ 1, 1, 1, 1 };
    }


    void Destroy(void) {
        m_colors.Clear();
    }
};


class Mesh : public AbstractMesh
{
public:
    TextureList         m_textures;
    VertexBuffer        m_vertices;
    VertexBuffer        m_normals;
    TexCoordBuffer      m_texCoords;
    ColorBuffer         m_vertexColors;
    IndexBuffer         m_indices;
    VAO                 m_vao;
    GLenum              m_shape;
    Vector3f            m_vMin;
    Vector3f            m_vMax;

    static uint32_t quadTriangleIndices[6];

    Mesh(bool isDynamic = true) {
        SetDynamic(isDynamic);
    }

    ~Mesh() {
        Destroy();
    }

    void Init(GLenum shape, int32_t listSegmentSize, Texture* texture = nullptr, String textureFolder = "", List<String> textureNames = List<String>(), GLenum textureType = GL_TEXTURE_2D);

    virtual void Create(int quality, Texture* texture, List<String> textureNames) {}

    virtual void Destroy(void);

    inline void SetDynamic(bool isDynamic) {
        m_vao.SetDynamic(isDynamic);
    }

    inline uint32_t ShapeSize(void) {
        if (m_shape == GL_QUADS)
            return 4;
        if (m_shape == GL_TRIANGLES)
            return 3;
        if (m_shape == GL_LINES)
            return 2;
        return 1;
    }

    inline void UpdateVertexBuffer(void) {
        m_vao.UpdateVertexBuffer("Vertex", m_vertices.GLData(), m_vertices.GLDataSize(), GL_FLOAT, 3);
    }

    inline void UpdateTexCoordBuffer(void) {
        m_vao.UpdateVertexBuffer("TexCoord", m_texCoords.GLData(), m_texCoords.GLDataSize(), GL_FLOAT, 2);
    }

    inline void UpdateColorBuffer(void) {
        m_vao.UpdateVertexBuffer("Color", m_vertexColors.GLData(), m_vertexColors.GLDataSize(), GL_FLOAT, 4);
    }
    // in the case of an icosphere, the vertices also are the vertex normals
    inline void UpdateNormalBuffer(void) {
        m_vao.UpdateVertexBuffer("Normal", m_normals.GLData(), m_normals.GLDataSize(), GL_FLOAT, 3);
    }

    inline void UpdateIndexBuffer(void) {
        m_vao.UpdateIndexBuffer(m_indices.GLData(), m_indices.GLDataSize(), GL_UNSIGNED_INT);
    }

    void UpdateVAO(bool createVertexIndex = false);

    void CreateVertexIndices(void);

    inline VAO& VAO(void) {
        return m_vao;
    }

    void SetupTexture(Texture* texture, String textureFolder, List<String> textureNames, GLenum textureType);

    virtual void PushTexture(Texture* texture);

    virtual void PopTexture(void);

    virtual Texture* GetTexture(void);

    bool EnableTexture(void);

    void DisableTexture(void);

    inline void AddVertex(Vector3f& v) {
        m_vertices.Append(v);
        m_vMin.Minimize(v);
        m_vMax.Maximize(v);
    }

    inline void AddTexCoord(SegmentedList<TexCoord>& tc) {
        m_texCoords.m_appData += tc;
    }

    inline void AddColor(RGBAColor& c) {
        m_vertexColors.Append(c);
    }

    inline void AddNormal(Vector3f& n) {
        m_normals.Append(n);
    }

    inline void AddIndices(ManagedArray<GLuint>& i) {
        m_indices.Append(i);
    }

    inline void SetIndices(ManagedArray<GLuint>& i) {
        m_indices.SetGLData(i);
    }

    inline bool IsEmpty(void) {
        return m_vertices.IsEmpty();
    }

    virtual void Render(Shader* shader, Texture* texture);
};

// =================================================================================================
