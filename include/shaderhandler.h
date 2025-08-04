#pragma once

#include "shader.h"
#include "shadercode.h"
#include "matrix.hpp"
//#include "gamedata.h"

// =================================================================================================

class ShaderHandler : public ShaderCode {
public:
    typedef Shader* (__cdecl* tShaderLoader) (void);

    ManagedArray<FloatArray*>  m_kernels;
    Shader*             m_activeShader;
    Texture             m_grayNoise;


    ShaderHandler() 
        : m_kernels(16)
    {
        m_activeShader = nullptr;
#if 0
        List<String> filenames = { appData->textureFolder + "graynoise.png" };
        m_grayNoise.CreateFromFile(filenames, appData->flipImagesVertically);
#endif
        ComputeGaussKernels();
        CreateShaders();
    }

    ~ShaderHandler() 
    { }

    Shader* SelectShader(Texture* texture);

    Shader* SetupShader(String shaderId);

    void StopShader(void);

    inline bool ShaderIsActive(Shader* shader = nullptr) {
        return m_activeShader != shader;
    }

    static int Compare(Shader* const& data, String const& key) {
        return String::Compare(nullptr, data->m_name, key);
    }

    inline Shader* GetShader(String shaderId) {
        //return m_shaders[shaderId];
        int32_t i = m_shaders.FindBinary(shaderId, ShaderHandler::Compare);
        return (i < 0) ? nullptr : m_shaders[i];
    }

    inline FloatArray* GetKernel(int radius) {
        return ((radius < 1) or (radius > m_kernels.Length())) ? nullptr : m_kernels[radius - 1];
    }

private:
    FloatArray* ComputeGaussKernel1D(int radius);

    void ComputeGaussKernels(void);
};

extern ShaderHandler* shaderHandler;

// =================================================================================================

