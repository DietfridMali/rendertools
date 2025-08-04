#include "basic_renderer.h"
#include "outlinerenderer.h"
#include "shaderhandler.h"

// =================================================================================================
// the(text) outline renderer works as follows :
// it uses render - to - texture with an FBO
// the FBO has three color buffers
// the text goes to the first color buffer.it must be sized down so that there is enough room around it for the outline
// the width of which is defined by the outlineWidth parameter
// the outline renderer uses buffer[0] as source and renders it to buffer[1], applying the outline shader in horizontal direction
// (i.e. the resulting text is widened horizontally)
// then buffer[1] is used as source and rendered to buffer[2], applying the vertical outline shader
// now buffer[2] contains a "fatter" version of the original text(buffer[0])
// render buffer[2] in the desired outline color and then buffer[2] on top of it : Voilà, there is your text with an outline


#define AUTORENDER 0

void OutlineRenderer::AntiAlias(FBO* fbo, OutlineRenderer::tAAMethod aaMethod) {
    if (aaMethod.method != "") {
        FBO::FBORenderParams params = { .clearBuffer = true, .scale = 1.0f };
        params.shader = shaderHandler->SetupShader(aaMethod.method);
        if (params.shader == nullptr)
            return;
        BasicRenderer::ClearGLError();
        params.shader->SetFloat("offset", 0.5f);
        if (aaMethod.method != "gaussblur")
            fbo->AutoRender(params);
        else {
            FloatArray* kernel = shaderHandler->GetKernel(aaMethod.strength);
            if (kernel != nullptr) {
                params.shader->SetFloatData("coeffs", *kernel);
                params.shader->SetInt("radius", aaMethod.strength);
                params.destination = fbo->GetLastDestination();
                
                //for (int i = 0; i < 1; i++) 
                {
                    // the following code only works if not called multiple times in a loop!
                    params.shader->SetFloat("direction", 0.0f);
                    params.source = params.destination;
                    params.destination = fbo->NextBuffer(params.source);
                    fbo->Render(params);
                    params.shader->SetFloat("direction", 1.0f);
                    params.source = params.destination;
                    params.destination = fbo->NextBuffer(params.source);
                    fbo->Render(params);
                    BasicRenderer::CheckGLError();
                }
            }
        }
        shaderHandler->StopShader();
    }
}


void OutlineRenderer::RenderOutline(FBO* fbo, float outlineWidth, RGBAColor color, tAAMethod aaMethod) {
    if (outlineWidth > 0) {
        Shader* shader = shaderHandler->SetupShader("outline");
        shader->SetFloat("outlineWidth", outlineWidth);
        shader->SetVector4f("outlineColor", color);
        shader->SetFloat("offset", 0.5f);
        fbo->AutoRender({ .clearBuffer = true, .shader = shader });
        shaderHandler->StopShader();
        AntiAlias(fbo, aaMethod);
    }
}

// =================================================================================================

