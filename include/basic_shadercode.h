#pragma once

#include "shader.h"
#include "string.hpp"

// =================================================================================================

class ShaderSource {
public:
    using KeyType = String;

    String m_name;
    String m_vs;
    String m_fs;

    ShaderSource() { }

    explicit ShaderSource(String name, String vs, String fs)
        : m_name(name), m_vs(vs), m_fs(fs)
    { }

    ShaderSource(const ShaderSource& other)
        : m_name(other.m_name), m_vs(other.m_vs), m_fs(other.m_fs)
    {
    }

    inline String& GetKey(void) {
        return m_name;
    }
#if 0    
    inline const bool operator< (ShaderSource const& other) {
        return m_name < other.m_name;
    }

    inline const bool operator> (ShaderSource const& other) {
        return m_name > other.m_name;
    }

    inline const bool operator<= (ShaderSource const& other) {
        return m_name <= other.m_name;
    }

    inline const bool operator>= (ShaderSource const& other) {
        return m_name >= other.m_name;
    }

    inline const bool operator!= (ShaderSource const& other) {
        return m_name != other.m_name;
    }

    inline const bool operator== (ShaderSource const& other) {
        return m_name == other.m_name;
    }
    
    static int Compare(const ShaderSource* o1, const ShaderSource* o2) {
        return String::Compare(nullptr, o1->m_name, o2->m_name);
    }
#endif
};

// =================================================================================================

extern String standardVS;

// =================================================================================================

class BasicShaderCode 
    : public Shader 
{
protected:
    ManagedArray<ShaderSource*> m_shaderSource;
    ManagedArray<Shader*>       m_shaders;

public:
    BasicShaderCode();

    void AddShaders(ManagedArray<ShaderSource*> shaderSource);

    void CreateShaders(void);

    static int Compare(Shader* const& data, String const& key) {
        return String::Compare(nullptr, data->m_name, key);
    }

    inline Shader* GetShader(String shaderId) {
        //return m_shaders[shaderId];
        int32_t i = m_shaders.FindBinary(shaderId, BasicShaderCode::Compare);
        return (i < 0) ? nullptr : m_shaders[i];
    }
};

// =================================================================================================

