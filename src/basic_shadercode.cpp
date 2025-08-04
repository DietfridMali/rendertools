
#include "array.hpp"
#include "string.hpp"
#include "basic_shadercode.h"

// =================================================================================================

extern ShaderSource plainColorShader;
extern ShaderSource plainTextureShader;
extern ShaderSource outlineShader;
extern ShaderSource boxBlurShader;
extern ShaderSource fxaaShader;
extern ShaderSource gaussBlurShader;

// -------------------------------------------------------------------------------------------------

BasicShaderCode::BasicShaderCode() {
    m_shaderSource = {
        &plainColorShader,
        &plainTextureShader,
        &outlineShader,
        &boxBlurShader,
        &fxaaShader,
        &gaussBlurShader
    };
}


void BasicShaderCode::AddShaders(ManagedArray<ShaderSource*> shaderSource) {
    m_shaderSource.Append(shaderSource, false);
}


void BasicShaderCode::CreateShaders(void) {
    std::sort(m_shaderSource.begin(), m_shaderSource.end(), [](const ShaderSource* a, const ShaderSource* b) { return a->m_name < b->m_name; }); // ascending
    m_shaders.Reserve(m_shaderSource.Length());
    for (int i = 0; i < m_shaderSource.Length(); i++) {
        ShaderSource* ss = m_shaderSource[i];
        fprintf(stderr, "creating shader '%s'", (char*) ss->m_name);
        Shader* s = new Shader(ss->m_name);
        if (s->Create(ss->m_vs, ss->m_fs))
            m_shaders.Push(s);
        else {
            m_shaders.Push(nullptr);
            fprintf(stderr, " - failed");
        }
        fprintf(stderr, "\n");
        //m_shaders.Insert(ss.m_name, s);
    }
}

// =================================================================================================
