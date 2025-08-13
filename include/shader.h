#pragma once

#include <type_traits>
#include <cstring>
#include <memory>

#include "glew.h"
#include "array.hpp"
#include "dictionary.hpp"
#include "vector.hpp"
#include "string.hpp"
#include "texture.h"
#include "shaderdata.h"

#define GLOBAL_UNIFORM_LOOKUP 0

// =================================================================================================
// Some basic shader handling: Compiling, enabling, setting shader variables

class Shader 
{
    public:
        GLuint          m_handle;
        String          m_name;
        String          m_vs;
        String          m_fs;
#if GLOBAL_UNIFORM_LOOKUP
        static Dictionary<String, UniformHandle*> uniforms;
#else
        ManagedArray<UniformHandle*> m_uniforms;
#endif
        static Dictionary<String, GLint> locations;

        using KeyType = String;

        Shader(String name = "", String vs = "", String fs = "") : 
            m_handle(0), m_name(name) 
        { }

        Shader(const Shader& other) {
            m_handle = other.m_handle;
        }

        Shader (Shader&& other) noexcept {
            m_handle = other.m_handle;
            other.m_handle = 0;
        }

        ~Shader () {
            Destroy ();
        }

        Shader& operator=(Shader&& other) noexcept {
            m_handle = other.m_handle;
            other.m_handle = 0;
            return *this;
        }

        String& GetKey(void) {
            return m_name;
        }

        String GetInfoLog (GLuint handle, bool isProgram = false);
            
        GLuint Compile(const char* code, GLuint type);

        GLuint Link(GLuint vsHandle, GLuint fsHandle);

        inline bool Create(const String& vsCode, const String& fsCode) {
            m_handle = Link(Compile((const char*)vsCode, GL_VERTEX_SHADER), Compile((const char*)fsCode, GL_FRAGMENT_SHADER));
            return m_handle != 0;
        }


        inline void Destroy(void) {
            if (m_handle > 0) {
                glDeleteProgram(m_handle);
                m_handle = 0;
            }
        }


        inline GLuint Handle(void) {
            return m_handle;
        }

#if 1
        inline GLint GetLocation(const char* name) const {
            GLint location;
            String key = String::Concat(m_name, "::", name);
            if (not locations.Find(key, location)) {
                location = glGetUniformLocation(m_handle, name);
                locations[key] = location;
            }
            return location;
        }
#endif

        template <typename T>
#if GLOBAL_LOOKUP
        inline T* GetUniform(const char* name) const {
            String key = String::Concat(m_name, "::", name);
            UniformHandle** uniformHandle = uniforms.Find(key);
            if (uniformHandle)
                return dynamic_cast<T*>(*uniformHandle);
            GLint location = glGetUniformLocation(m_handle, name);
            T* uniform = new T(name, location);
            uniforms[key] = uniform;
            return uniform;
        }
#else
        inline T* GetUniform(const char* name) {
            for (UniformHandle* uniformHandle : m_uniforms)
                if (uniformHandle->m_name == name)
                    return dynamic_cast<T*>(uniformHandle);
            GLint location = glGetUniformLocation(m_handle, name);
            T* uniform = new T(name, location);
            m_uniforms.Append(uniform);
            return uniform;
        }
#endif


        GLint SetMatrix4f(const char* name, const float* data, bool transpose = false);

        inline GLint SetMatrix4f(const char* name, ManagedArray<GLfloat>& data, bool transpose = false) {
            return SetMatrix4f(name, data.Data(), transpose);
        }

        GLint SetMatrix3f(const char* name, float* data, bool transpose = false);

        inline GLint SetMatrix3f(const char* name, ManagedArray<GLfloat>& data, bool transpose) {
            SetMatrix3f(name, data.Data(), transpose);
        }

        GLint SetVector4f(const char* name, const Vector4f& data);

        inline GLint SetVector4f(const char* name, Vector4f&& data) {
            return SetVector4f(name, static_cast<const Vector4f&>(data));
        }

        GLint SetVector3f(const char* name, const Vector3f& data);

        inline GLint SetVector3f(const char* name, Vector3f&& data) {
            return SetVector3f(name, static_cast<const Vector3f&>(data));
        }

        GLint SetVector2f(const char* name, const Vector2f& data);

        inline GLint SetVector2f(const char* name, Vector2f&& data) {
            return SetVector2f(name, static_cast<const Vector2f&>(data));
        }

        inline GLint SetVector2f(const char* name, float x, float y) {
            return SetVector2f(name, Vector2f(x, y));
        }

        GLint SetFloat(const char* name, float data);

        GLint SetVector2i(const char* name, const GLint* data);

        GLint SetVector3i(const char* name, const GLint* data);

        GLint SetVector4i(const char* name, const GLint* data);

        GLint SetInt(const char* name, int data);

        GLint SetFloatData(const char* name, const float* data, size_t length);

        inline GLint SetFloatData(const char* name, const FloatArray& data) {
            return SetFloatData(name, data.Data(), size_t(data.Length()));
        }

        GLint SetIntData(const char* name, const int* data, size_t length);

        inline GLint SetIntData(const char* name, const IntArray& data) {
            return SetIntData(name, data.Data(), size_t(data.Length()));
        }

        static inline float* GetFloatData(GLenum id, int32_t size, float* data) {
            glGetFloatv(id, (GLfloat*)data);
            return data;
        }

        static inline ManagedArray<float>& GetFloatData(GLenum id, int32_t size, ManagedArray<float>& glData) {
            if (glData.Length() < size)
                glData.Resize(size);
            GetFloatData(id, size, glData.Data());
            return glData;
        }

        inline void Enable(void) {
            glUseProgram(m_handle);
        }

        inline void Disable(void) {
            glUseProgram(0);
        }

        void UpdateMatrices(void);

        inline const bool operator< (String const& name) const { return m_name < name; }

        bool operator> (const String& name) const { return m_name > name; }

        bool operator<= (const String& name) const { return m_name <= name; }

        bool operator>= (const String& name) const { return m_name >= name; }

        bool operator!= (const String& name) const { return m_name != name; }

        bool operator== (const String& name) const { return m_name == name; }

        bool operator< (const Shader& other) const { return m_name < other.m_name; }

        bool operator> (const Shader& other) const { return m_name > other.m_name; }

        bool operator<= (const Shader& other) const { return m_name <= other.m_name; }

        bool operator>= (const Shader& other) const { return m_name >= other.m_name; }

        bool operator!= (const Shader& other) const { return m_name != other.m_name; }

        bool operator== (const Shader& other) const { return m_name == other.m_name; }
};

// =================================================================================================
