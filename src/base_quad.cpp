
#define NOMINMAX
#include <algorithm>

#include "vbo.h"
#include "vao.h"
#include "base_quad.h"
#include "base_shaderhandler.h"
#include "type_helper.hpp"

#define USE_VAO true

// caution: the VAO shared handle needs glGenVertexArrays and glDeleteVertexArrays, which usually are not yet available when this vao is initialized.
// VAO::Init takes care of that by first assigning a handle-less shared gl handle 
VAO* BaseQuad::m_vao = nullptr; 

// =================================================================================================

BaseQuad& BaseQuad::Copy(const BaseQuad& other) {
    if (this != &other) {
        m_vertexBuffer = other.m_vertexBuffer;
        m_texCoordBuffer = other.m_texCoordBuffer;
        m_texture = other.m_texture;
        m_color = other.m_color;
        m_aspectRatio = other.m_aspectRatio;
        m_isAvailable = other.m_isAvailable;
    }
    return *this;
}


BaseQuad& BaseQuad::Move(BaseQuad& other) {
    if (this != &other) {
        m_vertexBuffer = std::move(other.m_vertexBuffer);
        m_texCoordBuffer = std::move(other.m_texCoordBuffer);
        m_texture = std::move(other.m_texture);
        m_color = other.m_color;
        m_aspectRatio = other.m_aspectRatio;
        m_isAvailable = other.m_isAvailable;
    }
    return *this;
}


void BaseQuad::CreateTexCoords(void) {
    if (m_texCoordBuffer.AppDataLength() > 0) {
        for (auto& tc : m_texCoordBuffer.m_appData)
            m_maxTexCoord = TexCoord({ std::max(m_maxTexCoord.U(), tc.U()), std::max(m_maxTexCoord.V(), tc.V()) });
    }
    else {
        if (m_texture and (m_texture->WrapMode() == GL_REPEAT)) {
            m_maxTexCoord = TexCoord{ 0, 0 };
            for (auto& v : m_vertexBuffer.m_appData) {
                m_texCoordBuffer.Append(TexCoord({ v.X(), v.Z() }));
                m_maxTexCoord = TexCoord({ std::max(m_maxTexCoord.U(), v.X()), std::max(m_maxTexCoord.V(), v.Y()) });
            }
        }
        else {
            m_texCoordBuffer.Append(TexCoord{ 0, 1 });
            m_texCoordBuffer.Append(TexCoord{ 0, 0 });
            m_texCoordBuffer.Append(TexCoord{ 1, 0 });
            m_texCoordBuffer.Append(TexCoord{ 1, 1 });
            m_maxTexCoord = TexCoord{ 1, 1 };
        }
    }
}


bool BaseQuad::Setup(std::initializer_list<Vector3f> vertices, std::initializer_list<TexCoord> texCoords, Texture* texture, RGBAColor color/*, float borderWidth*/) {
    Plane::Init(vertices);
    m_vertexBuffer.m_appData = vertices;
    m_texCoordBuffer.m_appData = texCoords;
    CreateTexCoords();
    m_vertexBuffer.Setup();
    m_texCoordBuffer.Setup();
    if (not CreateVAO ())
        return false;
    m_vao->Init(GL_QUADS);
    m_texture = texture;
    m_color = color;
    m_aspectRatio = ComputeAspectRatio();
    return true;
}


bool BaseQuad::CreateVAO(void) {
    if (m_vao)
        return true;
    if (not (m_vao = new VAO(true)))
        return false;
    m_vao->Init(GL_QUADS);
    return true;
}


bool BaseQuad::UpdateVAO(void) {
    if (not CreateVAO())
        return false;
    if (m_vao->IsValid() and not m_vertexBuffer.m_appData.IsEmpty()) {
        m_vao->Enable();
        m_vao->UpdateVertexBuffer("Vertex", m_vertexBuffer.GLData(), m_vertexBuffer.GLDataSize(), GL_FLOAT, 3);
        m_vao->UpdateVertexBuffer("TexCoord", m_texCoordBuffer.GLData(), m_texCoordBuffer.GLDataSize(), GL_FLOAT, 2);
        m_vao->Disable();
    }
    return m_vao->IsValid();
}


float BaseQuad::ComputeAspectRatio(void) {
    Vector3f vMin = Vector3f{ 1e6, 1e6, 1e6 };
    Vector3f vMax = Vector3f{ -1e6, -1e6, -1e6 };
    for (auto& v : m_vertexBuffer.m_appData) {
        vMin.Minimize(v);
        vMax.Maximize(v);
    }
    return (vMax.Y() - vMin.Y()) / (vMax.X() - vMin.X());
}


Shader* BaseQuad::LoadShader(bool useTexture, const RGBAColor& color) {
    static ShaderLocationTable shaderLocations[2];
    String shaderNames[] = { "plainTexture", "plainColor" };
    int shaderId = useTexture ? 0 : 1;
    Shader* shader = baseShaderHandler.SetupShader(shaderNames[shaderId]);
    if (shader) {
        ShaderLocationTable& locations = shaderLocations[shaderId];
        locations.Start();
        shader->SetVector4f("surfaceColor", locations.Current(), color);
    }
    return shader;
}


void BaseQuad::Render(RGBAColor color) {
    if (UpdateVAO()) {
        Render(LoadShader(m_texture != nullptr, color), m_texture, false);
        //baseShaderHandler.StopShader();
    }
    else {
        glEnable(GL_TEXTURE_2D);
        m_texture->Enable();
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBegin(GL_QUADS);
        for (auto& v : m_vertexBuffer.m_appData) {
            glColor4f(color.R(), color.G(), color.B(), color.A());
            glVertex3f(v.X(), v.Y(), v.Z());
        }
        glEnd();
        m_texture->Disable();
    }
}


void BaseQuad::Render(Shader* shader, Texture* texture, bool updateVAO) {
    if (not updateVAO or UpdateVAO()) {
        m_vao->Render(shader, texture);
    }
    else {
        glEnable(GL_TEXTURE_2D);
        texture->Enable();
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBegin(GL_QUADS);
        for (auto& v : m_vertexBuffer.m_appData) {
            glColor4f(1, 1, 1, 1);
            glVertex3f(v.X(), v.Y(), v.Z());
        }
        glEnd();
        texture->Disable();
    }
}


void BaseQuad::Render(Texture* texture) {
    if (UpdateVAO()) {
        m_vao->Render(LoadShader(texture != nullptr), texture);
    }
}


// fill 2D area defined by x and y components of vertices with color color
void BaseQuad::Fill(RGBAColor color) {
    if (UpdateVAO()) {
        Render(LoadShader(false, color), nullptr, false);
        //baseShaderHandler.StopShader();
    }
    else {
        glDisable(GL_TEXTURE_2D);
        glBegin(GL_QUADS);
        //for (auto& v : m_vertexBuffer.m_appData) 
        {
            glColor4f(color.R(), color.G(), color.B(), color.A());
            //glVertex2f(v.X(), v.Y());

            glVertex2f(0, 0);
            glVertex2f(0, 1);
            glVertex2f(1, 1);
            glVertex2f(1, 0);

        }
        glEnd();
    }
}


void BaseQuad::Destroy(void) {
    if constexpr (not is_static_member_v<&BaseQuad::m_vao>) {
        m_vao->Destroy(); // don't destroy static members as they may be reused by other resources any time during program execution. Will be automatically destroyed at program termination
    }
    //textureHandler.Remove (m_texture); // BaseQuad textures are shared with quads and maybe reused after such a quad has been destroyed; so don't remove it globally
}

// =================================================================================================
