
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
    ManagedArray<ShaderSource*> shaderSource = {
        &plainColorShader,
        &plainTextureShader,
        &outlineShader,
        &boxBlurShader,
        &fxaaShader,
        &gaussBlurShader
    };
    AddShaders(shaderSource);
}


void BasicShaderCode::AddShaders(ManagedArray<ShaderSource*>& shaderSource) {
    for (ShaderSource* source : shaderSource) {
        fprintf(stderr, "creating shader '%s'", (char*) source->m_name);
        Shader* shader = new Shader(source->m_name);
        if (shader->Create(source->m_vs, source->m_fs))
            m_shaders[source->m_name] = shader;
        else
            fprintf(stderr, " - failed");
        fprintf(stderr, "\n");
        //m_shaders.Insert(ss.m_name, s);
    }
}

// =================================================================================================
