
#include "matrix.hpp"
#include "base_shaderhandler.h"
#include "base_renderer.h"

#define _USE_MATH_DEFINES
#include <math.h>

// =================================================================================================

FloatArray* BaseShaderHandler::ComputeGaussKernel1D(int radius) {
    FloatArray* kernel = new FloatArray(2 * radius + 1);

    const float sigma = float(radius) / 1.6f; // 2.0f; // Standardabweichung
    const float sigma2 = 2.0f * sigma * sigma;
    const float sqrtSigmaPi2 = float (std::sqrt(M_PI * sigma2));
    float sum = 0.0f;
#ifdef _DEBUG
    float k[33];
#endif

    for (int i = -radius; i <= radius; ++i) {
        float value = std::exp(-i * i / sigma2) / sqrtSigmaPi2;
        (*kernel)[i + radius] = value;
#ifdef _DEBUG
        k[i + radius] = value;
#endif
        sum += value;
    }

    // normalisation
    for (auto& value : *kernel)
        value /= sum;
#ifdef _DEBUG
    for (int i = 0; i < 2 * radius + 1; i++)
        k[i] /= sum;
#endif
    return kernel;
}


void BaseShaderHandler::ComputeGaussKernels(void) {
    for (int radius = 1; radius <= 16; radius++)
        m_kernels[radius - 1] = ComputeGaussKernel1D(radius);
}


Shader* BaseShaderHandler::SelectShader(Texture* texture) {
    String shaderId = "";
    if (not texture)
        shaderId = "color";
    // select shader depending on texture type
    else if (texture->Type() == GL_TEXTURE_CUBE_MAP)
        shaderId = "cubemap";
    else if (texture->Type() == GL_TEXTURE_2D)
        shaderId = "texture";
    else
        return nullptr;
    return SetupShader(shaderId);
}


Shader* BaseShaderHandler::SetupShader(String shaderId) {
    Shader* shader;
    if ((m_activeShaderId == shaderId) and (m_activeShader != nullptr))
        shader = m_activeShader;
    else {
        shader = GetShader(shaderId);
        //Shader** shaderPtr = m_shaders.Find(shaderId); // m_shaders[shaderId];
        if (shader == nullptr) {
            //fprintf(stderr, "*** couldn't find shader'%s'\r\n", (char*)shaderId);
            return nullptr;
        }
        //Shader* shader = *shaderPtr;
        if (shader->m_handle == 0) {
            fprintf(stderr, "*** shader'%s' is not available\r\n", (char*)shaderId);
            return nullptr;
        }
        //fprintf(stderr, "loading shader '%s'\r\n", (char*) shaderId);
        m_activeShader = shader;
        shader->Enable();
    }
    shader->UpdateMatrices();
    return shader;
}


void BaseShaderHandler::StopShader(bool needLegacyMatrices) {
    if (ShaderIsActive()) {
        m_activeShader->Disable();
        m_activeShader = nullptr;
        if (needLegacyMatrices)
            baseRenderer.UpdateLegacyMatrices();
    }
}

// =================================================================================================
