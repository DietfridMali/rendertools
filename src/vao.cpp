#include "vao.h"
#include "shaderhandler.h"

// =================================================================================================
// "Premium version of" OpenGL vertex array objects. CVAO instances offer methods to convert python
// lists into the corresponding lists of OpenGL items (vertices, normals, texture coordinates, etc)
// The current implementation requires a fixed order of array buffer creation to comply with the 
// corresponding layout positions in the shaders implemented here.
// Currently offers shaders for cubemap and regular (2D) texturing.
// Implements loading of varying textures, so an application item derived from or using a CVAO instance
// (e.g. an ico sphere) can be reused by different other application items that require different 
// texturing. This implementation basically allows for reusing one single ico sphere instance whereever
// a sphere is needed.
// Supports indexed and non indexed vertex buffer objects.
//
// // Due to the current shader implementation (fixed position layout), buffers need to be passed in a
// fixed sequence: vertices, colors, ...
// TODO: Expand shader for all kinds of inputs (texture coordinates, normals)
// See also https://qastack.com.de/programming/8704801/glvertexattribpointer-clarification

bool VAO::Init (GLuint shape) {
    m_shape = shape;
#if USE_SHARED_HANDLES
    if (m_handle.IsAvailable())
        return true;
    m_handle = SharedGLHandle(0, glGenVertexArrays, glDeleteVertexArrays); // need to set allocate and release functions
    return (m_handle.Claim() != 0);
#else
    if (m_handle)
        return true;
    glGenVertexArrays(1, &m_handle);
    return m_handle != 0;
#endif
}


void VAO::Destroy(void) {
    for (auto& vbo : m_dataBuffers)
        vbo->Destroy();
    m_indexBuffer.Destroy();
    m_dataBuffers.Clear();
#if USE_SHARED_HANDLES
    m_handle.Release();
#else
    if (m_handle) {
        glDeleteVertexArrays(1, &m_handle);
        m_handle = 0;
    }
#endif
}


VAO& VAO::Copy (VAO const& other) {
    if (this != &other) {
        m_dataBuffers = other.m_dataBuffers;
        m_indexBuffer = other.m_indexBuffer;
        m_handle = other.m_handle;
        m_shape = other.m_shape;
    }
    return *this;
}


VAO& VAO::Move(VAO& other) {
    if (this != &other) {
        m_dataBuffers = std::move(other.m_dataBuffers);
        m_indexBuffer = std::move(other.m_indexBuffer);
        m_handle = std::move(other.m_handle);
        m_shape = other.m_shape;
    }
    return *this;
}


VBO* VAO::FindBuffer(const char* type, int& index) {
    int i = 0;
    for (auto vbo : m_dataBuffers) {
        if (vbo->IsType(type)) {
            index = i;
            return vbo;
        }
        ++i;
    }
    return nullptr;
}

// add a vertex or index data buffer
bool VAO::UpdateBuffer(const char* type, void * data, size_t dataSize, size_t componentType, size_t componentCount) {
    if (strcmp(type, "Index"))
        return UpdateVertexBuffer(type, data, dataSize, componentType, componentCount);
    UpdateIndexBuffer(data, dataSize, componentType);
    return true;
}


bool VAO::UpdateVertexBuffer(const char* type, void * data, size_t dataSize, size_t componentType, size_t componentCount) {
    int index;
    VBO* vbo = FindBuffer(type, index);
    if (not vbo) { // otherwise index has been initialized by FindBuffer()
        vbo = new VBO();
        if (not vbo)
            return false;
        m_dataBuffers.Append(vbo);
        vbo->SetDynamic(m_isDynamic);
        index = m_dataBuffers.Length() - 1;
    }
    vbo->Update(type, GL_ARRAY_BUFFER, index, data, dataSize, componentType, componentCount);
    return true;
}


void VAO::UpdateIndexBuffer(void * data, size_t dataSize, size_t componentType) {
    m_indexBuffer.Update("Index", GL_ELEMENT_ARRAY_BUFFER, -1, data, dataSize, componentType);
}


void VAO::Render(Shader* shader, Texture* texture) {
#if 1
    if (shaderHandler->ShaderIsActive()) {
        EnableTexture(texture);
    }
#endif
    Enable();
    if (m_indexBuffer.m_data)
        glDrawElements(m_shape, m_indexBuffer.m_itemCount, m_indexBuffer.m_componentType, nullptr); // draw using an index buffer
    else
        glDrawArrays(m_shape, 0, m_dataBuffers[0]->m_itemCount); // draw non indexed arrays
    Disable();
#if 0
    if (shader != nullptr)
        shaderHandler->StopShader();
#endif
    DisableTexture(texture);
}

// =================================================================================================
