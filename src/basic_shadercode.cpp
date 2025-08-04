
#include "array.hpp"
#include "string.hpp"
#include "basic_shadercode.h"

// =================================================================================================

extern const ShaderSource& PlainColorShader();
extern const ShaderSource& PlainTextureShader();
extern const ShaderSource& OutlineShader();
extern const ShaderSource& BoxBlurShader();
extern const ShaderSource& FxaaShader();
extern const ShaderSource& GaussBlurShader();

// -------------------------------------------------------------------------------------------------

BasicShaderCode::BasicShaderCode() {
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


void BasicShaderCode::AddShaders(ManagedArray<const ShaderSource*>& shaderSource) {
    for (const ShaderSource* source : shaderSource) {
        fprintf(stderr, "creating shader '%s'", (const char*) source->m_name);
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
