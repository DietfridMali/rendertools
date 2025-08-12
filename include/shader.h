#pragma once

#include "glew.h"
#include "array.hpp"
#include "dictionary.hpp"
#include "vector.hpp"
#include "string.hpp"
#include "texture.h"
#include <cstring>

// =================================================================================================

struct UniformID
{
    GLint   m_location{};
    String  m_name{ "" };

    UniformID() = default;
    UniformID(String name, GLint location) : m_location{ location }, m_name{ std::move(name) } {}
    virtual ~UniformID() = default; // falls du polymorph speichern willst

    inline GLint& Location(void) { return m_location; }
    inline String& Name(void) { return m_name; }
};


template<typename T>
struct UniformVariable
    : public UniformID
{

    T m_value{};

    UniformVariable() = default;
    UniformVariable(String name, GLint location, T value = {})
        : UniformID(std::move(name), location), m_value(std::move(value)) {
    }

    // Zuweisung AUS einem T
    UniformVariable& operator=(const T& other) {
        m_value = other;
        return *this;
    }
    UniformVariable& operator=(T&& other) noexcept(std::is_nothrow_move_assignable_v<T>) {
        m_value = std::move(other);
        return *this;
    }

    // optional: Vergleich mit T
    inline bool operator==(const T& other) const noexcept { return m_value == other; }
    inline bool operator!=(const T& other) const noexcept { return !(*this == other); }

    inline T& Value(void) { return m_value; }
};


#include <type_traits>
#include <cstring>
#include <memory>
#include <GL/gl.h>

// angenommen: using String = std::string; // falls nicht bereits vorhanden

template<typename PointerType>
struct UniformArray : public UniformID {
    static_assert(std::is_pointer_v<PointerType>, "UniformArray only works with pointer types.");
    using BaseType = std::remove_pointer_t<PointerType>;

    std::unique_ptr<BaseType[]> m_value; // eigener Speicher
    size_t                      m_size{ 0 }; // Größe in BYTES
    size_t                      m_elems{ 0 };

    UniformArray() = default;

    bool Create(PointerType value, size_t size)
        if (not value or (size == 0)) {
            m_value.reset();
            m_size =
                m_elems = 0;
            return true;
        }

    if (size % sizeof(BaseType)) // size must be a multiple of sizeof(BaseType)
        return false;
    m_elems = size / sizeof(BaseType);

    if (not m_value or (m_size != size)) {
        m_value = std::make_unique<BaseType[]>(m_elems);
        m_size = bytes;
    }

    std::memcpy(m_value.get(), value, size);
    return true;
}

UniformArray(String name, GLint location, PointerType value = nullptr, size_t size = 0)
    : UniformID(std::move(name), location)
{
    Create(value, size);
}

// Setzen/Updaten via Funktionsaufruf-Operator:
// - Wenn Größe unterschiedlich: reallozieren + kopieren -> true zurück
// - Wenn Größe gleich: memcmp, falls anders -> kopieren -> true, sonst false
bool operator()(PointerType value, size_t size) {
    return Create(value, size);
}

// Vergleich mit externem Buffer + Bytegröße
bool operator==(PointerType other, size_t size) const noexcept {
    if (not m_value or not other)
        return false;
    if (m_size != size) // also catches size == 0
        return false;
    return std::memcmp(m_value.get(), other, size) == 0;
}


bool operator!=(PointerType other, size_t size) const noexcept {
    return !(*this == other, size); // Komma-Operator als Guard gegen versehentliche Klammerfehler
}

// Zugriff
inline PointerType Value() noexcept { return m_value.get(); }
inline const BaseType* Value() const noexcept { return m_value.get(); }
};

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

        static Dictionary<String, GLint>   locations;

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


        inline GLint GetLocation(const char* name) const {
            GLint location;
            String key = String::Concat(m_name, "::", name);
            if (not locations.Find(key, location)) {
                location = glGetUniformLocation(m_handle, name);
                locations[key] = location;
            }
            return location;
        }


        inline GLint SetMatrix4f(const char* name, const float* data, float* cachedData, bool transpose = false) const {
            GLint location = GetLocation(name);
            if (location >= 0) {
                if (not cachedData)
                    glUniformMatrix4fv(GetLocation(name), 1, GLboolean(transpose), data);
                else if (memcmp(cachedData, data, 16 * sizeof(float))) {
                    memcpy(cachedData, data, 16 * sizeof(float));
                    glUniformMatrix4fv(GetLocation(name), 1, GLboolean(transpose), data);
                }
            }
            return location;
        }


        inline GLint SetMatrix4f(const char* name, ManagedArray<GLfloat>& data, float* cachedData, bool transpose = false) {
            return SetMatrix4f(name, data.Data(), cachedData, transpose);
        }


        inline GLint SetMatrix3f(const char* name, float* data, bool transpose = false) {
            GLint location = GetLocation(name);
            if (location >= 0)
                glUniformMatrix3fv(location, 1, GLboolean(transpose), data);
            return location;
        }


        inline GLint SetMatrix3f(const char* name, ManagedArray<GLfloat>& data, bool transpose) {
            SetMatrix3f(name, data.Data(), transpose);
        }


        template<typename T>
        inline GLint SetVector4f(const char* name, T&& data) {
            GLint location = GetLocation(name);
            if (location >= 0)
                glUniform4fv(location, 1, std::forward<T>(data).Data());
            return location;
        }


        template<typename T>
        inline GLint SetVector3f(const char* name, T&& data) {
            GLint location = GetLocation(name);
            if (location >= 0)
                glUniform3fv(location, 1, std::forward<T>(data).Data());
            return location;
        }


        template<typename T>
        inline GLint SetVector2f(const char* name, T&& data) {
            GLint location = GetLocation(name);
            if (location >= 0)
                glUniform2fv(location, 1, std::forward<T>(data).Data());
            return location;
        }


        inline GLint SetVector2f(const char* name, float x, float y) {
            GLint location = GetLocation(name);
            if (location >= 0)
                glUniform2f(location, x, y);
            return location;
        }


        inline GLint SetFloat(const char* name, float data) {
            GLint location = GetLocation(name);
            if (location >= 0)
                glUniform1f(location, GLfloat(data));
            return location;
        }


        inline GLint SetVector2i(const char* name, const GLint* data) {
            GLint location = GetLocation(name);
            if (location >= 0)
                glUniform2iv(location, 1, data);
            return location;
        }


        inline GLint SetVector3i(const char* name, const GLint* data) {
            GLint location = GetLocation(name);
            if (location >= 0)
                glUniform3iv(location, 1, data);
            return location;
        }


        inline GLint SetVector4i(const char* name, const GLint* data) {
            GLint location = GetLocation(name);
            if (location >= 0)
                glUniform4iv(location, 1, data);
            return location;
        }


        inline GLint SetInt(const char* name, int data) {
            GLint location = GetLocation(name);
            if (location >= 0)
                glUniform1i(location, GLint(data));
            return location;
        }


        inline GLint SetFloatData(const char* name, FloatArray& data) {
            GLint location = GetLocation(name);
            if (location >= 0)
                glUniform1fv(location, GLsizei(data.Length()), (GLfloat*)data.Data());
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
