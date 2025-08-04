#pragma once

#include "glew.h"
#include "array.hpp"
#include "dictionary.hpp"
#include "vector.hpp"
#include "string.hpp"
#include "texture.h"

// =================================================================================================
// Some basic shader handling: Compiling, enabling, setting shader variables

class Shader 
{
    public:
        GLuint          m_handle;
        String          m_name;
        String          m_vs;
        String          m_fs;

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


        inline GLint GetLocation(const char* name) const {
            return glGetUniformLocation(m_handle, name);
        }


        inline void SetMatrix4f(const char* name, const float* data, bool transpose = false) const {
            glUniformMatrix4fv(GetLocation(name), 1, GLboolean(transpose), data);
        }


        inline void SetMatrix4f(const char* name, ManagedArray<GLfloat>& data, bool transpose = false) {
            SetMatrix4f(name, data.Data(), transpose);
        }


        inline void SetMatrix3f(const char* name, float* data, bool transpose = false) {
            glUniformMatrix3fv(GetLocation(name), 1, GLboolean(transpose), data);
        }


        inline void SetMatrix3f(const char* name, ManagedArray<GLfloat>& data, bool transpose) {
            SetMatrix3f(name, data.Data(), transpose);
        }


        template<typename T>
        inline void SetVector4f(const char* name, T&& data) {
            glUniform4fv(GetLocation(name), 1, std::forward<T>(data).Data());
        }


        template<typename T>
        inline void SetVector3f(const char* name, T&& data) {
            glUniform3fv(GetLocation(name), 1, std::forward<T>(data).Data());
        }


        template<typename T>
        inline void SetVector2f(const char* name, T&& data) {
            glUniform2fv(GetLocation(name), 1, std::forward<T>(data).Data());
        }


        inline void SetVector2f(const char* name, float x, float y) {
            glUniform2f(GetLocation(name), x, y);
        }


        inline void SetFloat(const char* name, float data) {
            glUniform1f(GetLocation(name), GLfloat(data));
        }


        inline void SetVector2i(const char* name, const GLint* data) {
            glUniform2iv(GetLocation(name), 1, data);
        }


        inline void SetVector3i(const char* name, const GLint* data) {
            glUniform3iv(GetLocation(name), 1, data);
        }


        inline void SetVector4i(const char* name, const GLint* data) {
            glUniform4iv(GetLocation(name), 1, data);
        }


        inline void SetInt(const char* name, int data) {
            glUniform1i(GetLocation(name), GLint(data));
        }


        void SetFloatData(const char* name, FloatArray& data) {
            glUniform1fv(GetLocation(name), GLsizei(data.Length()), (GLfloat*)data.Data());
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
