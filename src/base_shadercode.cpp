
#include "array.hpp"
#include "string.hpp"
#include "base_shadercode.h"

// =================================================================================================

const ShaderSource& PlainColorShader();
const ShaderSource& PlainTextureShader();
const ShaderSource& OutlineShader();
const ShaderSource& BoxBlurShader();
const ShaderSource& FxaaShader();
const ShaderSource& GaussBlurShader();

// -------------------------------------------------------------------------------------------------

BaseShaderCode::BaseShaderCode() {
    ManagedArray<const ShaderSource*> shaderSource = {
        &PlainColorShader(),
        &PlainTextureShader(),
        &OutlineShader(),
        &BoxBlurShader(),
        &FxaaShader(),
        &GaussBlurShader()
    };
    AddShaders(shaderSource);
}


void BaseShaderCode::AddShaders(ManagedArray<const ShaderSource*>& shaderSource) {
    for (const ShaderSource* source : shaderSource) {
        Shader* shader = new Shader(source->m_name);
        if (shader->Create(source->m_vs, source->m_fs))
            m_shaders[source->m_name] = shader;
        else
            fprintf(stderr, "creating shader '%s' failed\n", (const char*) source->m_name);
    }
}

// =================================================================================================
