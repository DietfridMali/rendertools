#pragma once

#include "shader.h"
#include "basic_shadercode.h"
#include "matrix.hpp"
//#include "gamedata.h"

// =================================================================================================

class BasicShaderHandler {
public:
    typedef Shader* (__cdecl* tShaderLoader) (void);

    ManagedArray<FloatArray*>   m_kernels;
    Shader*                     m_activeShader;
    Texture                     m_grayNoise;
    BasicShaderCode*            m_shaderCode;


    BasicShaderHandler() 
        : m_kernels(16), m_shaderCode(nullptr)
    {
        m_activeShader = nullptr;
#if 0
        List<String> filenames = { appData->textureFolder + "graynoise.png" };
        m_grayNoise.CreateFromFile(filenames, appData->flipImagesVertically);
#endif
        ComputeGaussKernels();
    }

    virtual ~BasicShaderHandler() {
        if (m_shaderCode)
            delete m_shaderCode; // Speicherbereinigung nicht vergessen!
    }

protected:
    virtual void CreateShaderCode(void) {  m_shaderCode = new BasicShaderCode(); }

public:
    int CreateShaders(void) {
        if (m_shaderCode != nullptr)
            return 0;
        CreateShaderCode();
        if (m_shaderCode == nullptr)
            return -1;
        m_shaderCode->CreateShaders();
        return 1;
    }

    Shader* SelectShader(Texture* texture);

    Shader* SetupShader(String shaderId);

    void StopShader(void);

    inline bool ShaderIsActive(Shader* shader = nullptr) {
        return m_activeShader != shader;
    }

    inline Shader* GetShader(String shaderId) {
        return m_shaderCode->GetShader (shaderId);
    }

    inline FloatArray* GetKernel(int radius) {
        return ((radius < 1) or (radius > m_kernels.Length())) ? nullptr : m_kernels[radius - 1];
    }

private:
    FloatArray* ComputeGaussKernel1D(int radius);

    void ComputeGaussKernels(void);
};

extern BasicShaderHandler* basicShaderHandler;

// =================================================================================================

