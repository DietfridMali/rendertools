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

// =================================================================================================
// Some basic shader handling: Compiling, enabling, setting shader variables

class Shader 
{
    public:
        GLuint          m_handle;
        String          m_name;
        String          m_vs;
        String          m_fs;
        float           m_modelView[16];
        float           m_baseModelView[16];
        float           m_projection[16];

        static Dictionary<String, UniformID*> uniforms;

        using KeyType = String;

        Shader(String name = "", String vs = "", String fs = "") : 
            m_handle(0), m_name(name) 
        { 
            memset(m_modelView, 0, sizeof(m_modelView));
            memset(m_baseModelView, 0, sizeof(m_modelView));
            memset(m_projection, 0, sizeof(m_projection));
        }

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

        inline float* ModelViewCache(void) { return m_modelView; }

        inline float* BaseModelViewCache(void) { return m_baseModelView; }

        inline float* ProjectionCache(void) { return m_projection; }

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

#if 0
        inline GLint GetLocation(const char* name) const {
            GLint location;
            String key = String::Concat(m_name, "::", name);
            if (not uniforms.Find(key, uniform)) {
                location = glGetUniformLocation(m_handle, name);
                locations[key] = location;
            }
            return location;
        }
#endif

        template <typename T>
        inline T* GetUniform(const char* name) const {
            T* uniform;
            String key = String::Concat(m_name, "::", name);
            if (not uniforms.Find(key, uniform)) {
                uniform.location = glGetUniformLocation(m_handle, name);
                uniform = new T(location, name);
                    uniforms[key] = uniform;
            }
            return uniform;
        }


        inline GLint SetMatrix4f(const char* name, const float* data, bool transpose = false) const {
            UniformArray16f* uniform = GetUniform<UniformArray16f>(name);
            if (not uniform or (uniform->location < 0))
                return -1;
            if (*uniform != data) {
                *uniform = data;
                glUniformMatrix4fv(GetLocation(name), 1, GLboolean(transpose), data);
            }
            return uniform->location;
        }


        inline GLint SetMatrix4f(const char* name, ManagedArray<GLfloat>& data, bool transpose = false) {
            return SetMatrix4f(name, data.Data(), transpose);
        }


        inline GLint SetMatrix3f(const char* name, float* data, bool transpose = false) {
            UniformArray9f* uniform = GetUniform<UniformArray9f>(name);
            if (not uniform or (uniform->location < 0))
                return -1;
            if (*uniform != data) {
                *uniform = data;
                glUniformMatrix3fv(location, 1, GLboolean(transpose), data);
            }
            return uniform->location;
        }


        inline GLint SetMatrix3f(const char* name, ManagedArray<GLfloat>& data, bool transpose) {
            SetMatrix3f(name, data.Data(), transpose);
        }


        template<typename T>
        inline GLint SetVector4f(const char* name, T&& data) {
            UniformVector4f* uniform = GetUniform<UniformVector4f>(name);
            if (not uniform or (uniform->location < 0))
                return -1;
            if (*uniform != data) {
                *uniform = data;
                glUniform4fv(location, 1, std::forward<T>(data).Data());
            }
            return uniform->location;
        }


        template<typename T>
        inline GLint SetVector3f(const char* name, T&& data) {
            UniformVector3f* uniform = GetUniform<UniformVector3f>(name);
            if (not uniform or (uniform->location < 0))
                return -1;
            if (*uniform != data) {
                *uniform = data;
                glUniform4fv(location, 1, std::forward<T>(data).Data());
            }
            return uniform->location;
        }


        template<typename T>
        inline GLint SetVector2f(const char* name, T&& data) {
            UniformVector2f* uniform = GetUniform<UniformVector2f>(name);
            if (not uniform or (uniform->location < 0))
                return -1;
            if (*uniform != data) {
                *uniform = data;
                glUniform4fv(location, 1, std::forward<T>(data).Data());
            }
            return uniform->location;
        }


        inline GLint SetVector2f(const char* name, float x, float y) {
            return SetVector2f(name, Vector2f(x, y));
        }


        inline GLint SetFloat(const char* name, float data) {
            UniformFloat* = GetUniform<UniformFloat>(name);
            if (not uniform or (uniform->location < 0))
                return -1;
            if (*uniform != data) {
                *uniform = data;
                glUniform1f(location, GLfloat(data));
            }
            return uniform->location;
        }


        inline GLint SetVector2i(const char* name, const GLint* data) {
            UniformArray2i* uniform = GetUniform<UniformArray2i>(name);
            if (not uniform or (uniform->location < 0))
                return -1;
            if (*uniform != data) {
                *uniform = data;
                glUniform2iv(location, 1, data);
            }
            return uniform->location;
        }


        inline GLint SetVector3i(const char* name, const GLint* data) {
            UniformArray3i* uniform = GetUniform<UniformArray3i>(name);
            if (not uniform or (uniform->location < 0))
                return -1;
            if (*uniform != data) {
                *uniform = data;
                glUniform3iv(location, 1, data);
            }
            return uniform->location;
        }


        inline GLint SetVector4i(const char* name, const GLint* data) {
            UniformArray4i* uniform = GetUniform<UniformArray4i>(name);
            if (not uniform or (uniform->location < 0))
                return -1;
            if (*uniform != data) {
                *uniform = data;
                glUniform4iv(location, 1, data);
            }
            return uniform->location;
        }


        inline GLint SetInt(const char* name, int data) {
            UniformInt* uniform = GetUniform<UniformInt>(name);
            if (not uniform or (uniform->location < 0))
                return -1;
            if (*uniform != data) {
                *uniform = data;
                glUniform1i(location, GLint(data));
            }
            return uniform->location;
        }


        inline GLint SetFloatData(const char* name, FloatArray& data) {
            UniformArray<float>* uniform = location = GetUniform<UniformArray<float>>(name);
            if (not uniform or (uniform->location < 0))
                return -1;
            if (*uniform != data) {
                * uniform = UniformArrayData(data, data.Length());
                glUniform1fv(location, GLsizei(data.Length()), (GLfloat*)data.Data());
            }
            return location;
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


        inline const bool operator< (String const& name) const {
            return m_name < name;
        }

        bool operator> (const String& name) const {
            return m_name > name;
        }

        bool operator<= (const String& name) const {
            return m_name <= name;
        }

        bool operator>= (const String& name) const {
            return m_name >= name;
        }

        bool operator!= (const String& name) const {
            return m_name != name;
        }

        bool operator== (const String& name) const {
            return m_name == name;
        }

        bool operator< (const Shader& other) const {
            return m_name < other.m_name;
        }

        bool operator> (const Shader& other) const {
            return m_name > other.m_name;
        }

        bool operator<= (const Shader& other) const {
            return m_name <= other.m_name;
        }

        bool operator>= (const Shader& other) const {
            return m_name >= other.m_name;
        }

        bool operator!= (const Shader& other) const {
            return m_name != other.m_name;
        }

        bool operator== (const Shader& other) const {
            return m_name == other.m_name;
        }
};

// =================================================================================================
