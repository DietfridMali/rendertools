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

#define GRADIENT R"(
        float Gradient() { 
            if (maxDist == 0.0)
                return 1.0;
            float s = clamp(pow (length(fragPos) / abs(maxDist), 0.5), 0.2, 1.0);
            return (maxDist > 0) ? 1.0 - s : 2.0 - 2 * s; // continually darker from start or gradient flipping in the middle
            }
        )"


#define IS_BORDER R"(
        bool IsBorder() {
            if (borderWidth == 0.0)
                return false;
            vec2 borders = borderWidth * vec2(aspectRatio, 1.0);
            return (min (fragTexCoord.x, maxTexCoord.x - fragTexCoord.x) < borders.x) || (min (fragTexCoord.y, maxTexCoord.y - fragTexCoord.y) < borders.y);
            }
        )"

extern String standardVS;

// =================================================================================================

class BasicShaderCode 
    : public Shader 
{
protected:
    ManagedArray<ShaderSource*> m_shaderSource;
    ManagedArray<Shader*>       m_shaders;

    BasicShaderCode();

    void AddShaders(ManagedArray<ShaderSource*> shaderSource);

    void CreateShaders(void);
};

// =================================================================================================

