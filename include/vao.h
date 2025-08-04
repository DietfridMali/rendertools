#pragma once

#include "glew.h"
#include "list.hpp"
#include "sharedglhandle.hpp"
#include "vbo.h"
#include "vector.hpp"
#include "texture.h"
#include "shader.h"

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

#ifdef USE_SHARED_HANDLES
#   undef USE_SHARED_HANDLES
#endif

#define USE_SHARED_HANDLES 1

class VAO 
{
    public:
        List<VBO*>      m_dataBuffers;
        VBO             m_indexBuffer;
#if USE_SHARED_HANDLES
        SharedGLHandle  m_handle;
#else
        GLuint          m_handle;
#endif
        GLuint          m_shape;
        bool            m_isDynamic;

        VAO(bool isDynamic = true)
            : m_isDynamic(isDynamic)
#if USE_SHARED_HANDLES
            , m_handle (SharedGLHandle(0, glGenVertexArrays, glDeleteVertexArrays))
#else
            , m_handle(0)
#endif
            , m_shape (0)
        { 
            SetDynamic(isDynamic);
        }

        inline void SetDynamic(bool isDynamic) {
            m_isDynamic = isDynamic;
            for (auto vbo : m_dataBuffers)
                vbo->SetDynamic(isDynamic);
            m_indexBuffer.SetDynamic(m_isDynamic);
        }

        bool Init (GLuint shape);

        ~VAO () {
            Destroy ();
        }

        VAO(VAO const& other) {
            Copy (other);
        }

        VAO(VAO&& other) noexcept {
            Move(other);
        }

        VAO& operator=(const VAO& other) {
            return Copy(other);
        }

        VAO& operator=(VAO&& other) noexcept {
            return Move(other);
        }

        VAO& Copy (VAO const& other);

        VAO& Move(VAO& other);

        void Destroy(void);

        void Reset (void);

        inline bool IsValid(void) {
#if USE_SHARED_HANDLES
            return m_handle.IsAvailable();
#else
            return m_handle != 0;
#endif
        }

        inline void Enable(void) {
#if USE_SHARED_HANDLES
            glBindVertexArray(m_handle);
#else
            glBindVertexArray(m_handle);
#endif
        }

        inline void Disable(void) {
            glBindVertexArray(0);
        }

        inline Texture* EnableTexture(Texture* texture) {
            if (texture != nullptr)
                texture->Enable();
            return texture;
        }

        inline void DisableTexture(Texture* texture) {
            if (texture != nullptr)
                texture->Disable();
        }


        VBO* FindBuffer(const char* type, int& index);

        // add a vertex or index data buffer
        bool UpdateBuffer(const char* type, void* data, size_t dataSize, size_t componentType, size_t componentCount = 0);

        bool UpdateVertexBuffer(const char* type, void* data, size_t dataSize, size_t componentType, size_t componentCount);

        void UpdateIndexBuffer(void* data, size_t dataSize, size_t componentType);

        void Render(Shader* shader, Texture* texture = nullptr);

};

// =================================================================================================
