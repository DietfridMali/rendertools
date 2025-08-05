#define NOMINMAX

#include <stdlib.h>
#include <algorithm>
#include <utility>

#include "conversions.hpp"
#include "glew.h"
//#include "quad.h"
#include "base_renderer.h"

// =================================================================================================
// basic renderer class. Initializes display && OpenGL && sets up projections && view transformation
// the renderer enforces window width >= window height, so for portrait screen mode, the window contents
// rendered sideways. That's why BaseRenderer class has m_windowWidth, m_windowHeight and m_aspectRatio
// separate from DisplayHandler.

void BaseRenderer::Init(int width, int height, float fov) {
    m_sceneWidth =
        m_windowWidth = width; // (width > height) ? width : height;
    m_sceneHeight =
        m_windowHeight = height; // (height > width) ? width : height;
    m_sceneLeft = 0;

    m_aspectRatio = float(m_windowWidth) / float(m_windowHeight); // just for code clarity
    m_drawBuffers.Resize(1);
    m_drawBuffers[0] = GL_BACK;
    ResetDrawBuffers(nullptr); // required to initialize m_drawBufferInfo. If not done here, subsequent renders to FBOs ahead of main rendering loop will crash the app
    CreateMatrices(m_windowWidth, m_windowHeight, float(m_sceneWidth) / float(m_sceneHeight), fov);
}


void BaseRenderer::Create(int width, int height, float fov) {
    Init(width, height, fov);
    m_viewport = ::Viewport(0, 0, m_windowWidth, m_windowHeight);
    SetupOpenGL();
    m_screenBuffer.Create(m_windowWidth, m_windowHeight, 1, { .name = "screen", .colorBufferCount = 1 }); // FBO for entire screen incl. 2D elements (e.g. UI)
    m_drawBufferStack.Clear();
    m_renderTexture.HasBuffer() = true;
    m_viewportArea.Setup({ Vector3f{ -0.5f, -0.5f, 0.0f }, Vector3f{ -0.5f, 0.5f, 0.0f }, Vector3f{ 0.5f, 0.5f, 0.0f }, Vector3f{ 0.5f, -0.5f, 0.0f } });
}


void BaseRenderer::SetupOpenGL (void) {
    glClearColor(0, 0, 0, 0);
    glColorMask (1, 1, 1, 1);
    glDepthMask (1);
    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LESS);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable (GL_ALPHA_TEST);
    glFrontFace(GL_CW);
    glEnable (GL_CULL_FACE);
    glCullFace (GL_BACK);
    glEnable (GL_MULTISAMPLE);
    glDisable (GL_POLYGON_OFFSET_FILL);
    glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    SetViewport ();
}


bool BaseRenderer::Start3DScene(void) {
    if (not m_sceneBuffer.IsAvailable())
        return false;
    ResetDrawBuffers(&m_sceneBuffer);
    SetupTransformation();
    SetupOpenGL();
    SetViewport(::Viewport(m_sceneLeft, 0, m_sceneWidth, m_sceneHeight), false);
    EnableCamera();
    return true;
}


bool BaseRenderer::Stop3DScene(void) {
    if (not m_sceneBuffer.IsAvailable())
        return false;
    DisableCamera();
    ResetTransformation();
    return true;
}


bool BaseRenderer::Start2DScene(void) {
    if (not m_screenBuffer.IsAvailable())
        return false;
    ResetDrawBuffers(&m_screenBuffer, not m_screenIsAvailable);
    m_screenIsAvailable = true;
    ResetTransformation();
    return true;
}


bool BaseRenderer::Stop2DScene(void) {
    if (not m_screenIsAvailable)
        return false;
    ResetDrawBuffers(nullptr);
    return true;
}


void BaseRenderer::Draw3DScene(void) {
    if (m_sceneBuffer.IsAvailable()) {
        Stop3DScene();
        Start2DScene();
        baseRenderer.PushMatrix();
        baseRenderer.Translate(0.5, 0.5, 0);
        baseRenderer.Scale(1, -1, 1);
        glDepthFunc(GL_ALWAYS);
        glDisable(GL_CULL_FACE);
        SetViewport(::Viewport(m_sceneLeft, 0, m_sceneWidth, m_sceneHeight), false);
#if 1
        m_renderTexture.m_handle = m_sceneBuffer.BufferHandle(0);
        m_viewportArea.Render(&m_renderTexture);
#else
        m_viewportArea.Fill(ColorData::Orange);
#endif
        baseRenderer.PopMatrix();
    }
}


void BaseRenderer::DrawScreen (bool bRotate) {
    if (m_screenIsAvailable) {
        Stop2DScene();
        m_screenIsAvailable = false;
        glDepthFunc(GL_ALWAYS);
        glDisable(GL_CULL_FACE); // required for vertical flipping because that inverts the buffer's winding
        SetViewport(::Viewport(0, 0, m_windowWidth, m_windowHeight), false);
        glClear(GL_COLOR_BUFFER_BIT);
        Translate(0.5, 0.5, 0);
#if 1
        if (bRotate)
            Rotate(90, 0, 0, 1);
#if 0
        else
            Scale(1, -1, 1);
#endif
        m_renderTexture.m_handle = m_screenBuffer.BufferHandle(0);
        m_viewportArea.Render(&m_renderTexture);
#else
        m_viewportArea.Fill(ColorData::Orange);
#endif
    }
}


void BaseRenderer::SetViewport(bool isFBO) {
    SetViewport(m_viewport, isFBO);
}


void BaseRenderer::SetViewport(::Viewport viewport, bool flipVertically) {
    m_viewport = viewport;
    if (flipVertically)
        glViewport(viewport.m_left, m_windowHeight - viewport.m_top - viewport.m_height, viewport.m_width, viewport.m_height);
    else
        glViewport(viewport.m_left, viewport.m_top, viewport.m_width, viewport.m_height);
}


bool BaseRenderer::SetActiveBuffer(FBO* buffer, bool clearBuffer) {
    if (m_activeBuffer != buffer) {
        if (m_activeBuffer)
            m_activeBuffer->Disable();
        m_activeBuffer = buffer;
    }
    if (not m_activeBuffer)
        return false;
    return m_activeBuffer->IsEnabled() or m_activeBuffer->Enable(0, clearBuffer);
}


void BaseRenderer::ResetDrawBuffers(FBO* activeBuffer, bool clearBuffer) {
    DrawBufferInfo info;
    while (m_drawBufferStack.Pop(info)) {
        if (info.m_fbo)
            info.m_fbo->Disable();
    }
    m_drawBufferInfo = DrawBufferInfo(nullptr, &m_drawBuffers); // &m_drawBuffer, m_drawBuffer.m_colorBufferInfo[0].m_attachment);
    if (not SetActiveBuffer(activeBuffer, clearBuffer))
        glDrawBuffers(m_drawBufferInfo.m_drawBuffers->Length(), m_drawBufferInfo.m_drawBuffers->Data());
}


void BaseRenderer::SaveDrawBuffer() {
    m_drawBufferStack.Push(m_drawBufferInfo);
}


void BaseRenderer::SetDrawBuffers(FBO* fbo, ManagedArray<GLuint>* drawBuffers) {
    if ((fbo != nullptr) && (m_drawBufferInfo.m_fbo != nullptr) && (fbo->m_handle == m_drawBufferInfo.m_fbo->m_handle))
        m_drawBufferInfo.m_drawBuffers = drawBuffers;
    else {
        SaveDrawBuffer();
        m_drawBufferInfo = DrawBufferInfo(fbo, drawBuffers);
    }
    ClearGLError();
    glDrawBuffers(m_drawBufferInfo.m_drawBuffers->Length(), m_drawBufferInfo.m_drawBuffers->Data());
    CheckGLError("setting draw buffer");
}


void BaseRenderer::RestoreDrawBuffer(void) {
    m_drawBufferStack.Pop(m_drawBufferInfo);
    glBindTexture(GL_TEXTURE_2D, 0);
    if (m_drawBufferInfo.m_fbo != nullptr)
        m_drawBufferInfo.m_fbo->Reenable();
    else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDrawBuffers(m_drawBufferInfo.m_drawBuffers->Length(), m_drawBufferInfo.m_drawBuffers->Data());
    }
}


void BaseRenderer::Fill(const RGBColor& color, float alpha, float scale) {
    baseRenderer.PushMatrix();
    baseRenderer.Translate(0.5, 0.5, 0.0);
    baseRenderer.Scale(scale, scale, 1);
    m_viewportArea.Fill(color, alpha);
    baseRenderer.PopMatrix();
}


void BaseRenderer::ClearGLError(void) {
    while (glGetError() != GL_NO_ERROR)
        ;
}


bool BaseRenderer::CheckGLError (const char* operation) {
    GLenum glError = glGetError ();
    if (glError) {
        fprintf (stderr, "OpenGL Error %d (%s)\n", glError, operation);
        return false;
    }
    return true;
}

BaseRenderer* baseRendererInstance = nullptr;

#define baseRenderer BaseRenderer::Instance()

// =================================================================================================
