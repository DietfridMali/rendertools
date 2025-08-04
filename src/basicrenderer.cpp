#define NOMINMAX

#include <stdlib.h>
#include <algorithm>
#include <utility>

#include "conversions.hpp"
#include "glew.h"
//#include "quad.h"
#include "basicrenderer.h"

// =================================================================================================
// basic renderer class. Initializes display && OpenGL && sets up projections && view transformation
// the renderer enforces window width >= window height, so for portrait screen mode, the window contents
// rendered sideways. That's why BasicRenderer class has m_windowWidth, m_windowHeight and m_aspectRatio
// separate from DisplayHandler.

BasicRenderer::BasicRenderer(int width, int height) // , Viewer* viewer)
    : m_activeBuffer(nullptr), m_screenIsValid(false)
{
    //m_viewer = viewer;
    m_sceneWidth =
    m_windowWidth = (width > height) ? width : height;
    m_sceneHeight =
    m_windowHeight = (height > width) ? width : height;
    m_sceneLeft = 0;

    m_aspectRatio = float(m_windowWidth) / float(m_windowHeight); // just for code clarity
    m_drawBuffers.Resize(1);
    m_drawBuffers[0] = GL_BACK;
    CreateMatrices(m_windowWidth, m_windowHeight, float(m_sceneWidth) / float(m_sceneHeight));
}


void BasicRenderer::Create(void) {
    m_viewport = ::Viewport(0, 0, m_windowWidth, m_windowHeight);
    SetupOpenGL();
    m_sceneBuffer.Create(m_sceneWidth, m_sceneHeight, 1, { .colorBufferCount = 1, .depthBufferCount = 1, .vertexBufferCount = 2, .hasMRTs = false } ); // FBO for 3D scene: 2 color (1 for normals plus transparency info in alpha channel), 1 depth, 1 vertex buffer (world pos)
    m_screenBuffer.Create(m_windowWidth, m_windowHeight, 1, { .colorBufferCount = 1 }); // FBO for entire screen incl. 2D elements (e.g. UI)
    m_drawBufferStack.Clear();
    m_renderTexture.HasBuffer() = true;
    m_viewportArea.Setup({ Vector3f{ -0.5f, -0.5f, 0.0f }, Vector3f{ -0.5f, 0.5f, 0.0f }, Vector3f{ 0.5f, 0.5f, 0.0f }, Vector3f{ 0.5f, -0.5f, 0.0f } });
}


void BasicRenderer::SetupOpenGL (void) {
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


void BasicRenderer::ResetDrawBuffers(FBO* activeBuffer, bool clearBuffer) {
    DrawBufferInfo info;
    while (m_drawBufferStack.Pop(info)) {
        if (info.m_fbo)
            info.m_fbo->Disable();
    }
    m_drawBufferInfo = DrawBufferInfo(nullptr, &m_drawBuffers); // &m_drawBuffer, m_drawBuffer.m_colorBufferInfo[0].m_attachment);
    SetActiveBuffer(activeBuffer, clearBuffer);
}


void BasicRenderer::Start3DScene(void) {
    ResetDrawBuffers(&m_sceneBuffer);
    SetViewport(::Viewport(m_sceneLeft, 0, m_sceneWidth, m_sceneHeight), false);
    SetupOpenGL();
    SetupTransformation();
    EnableCamera();
}


void BasicRenderer::Stop3DScene(void) {
    DisableCamera();
    ResetTransformation();
}


void BasicRenderer::Start2DScene(void) {
    ResetDrawBuffers(&m_screenBuffer, not m_screenIsValid);
    m_screenIsValid = true;
    ResetTransformation();
}


void BasicRenderer::Stop2DScene(void) {
    ResetDrawBuffers(nullptr);
}


void BasicRenderer::Draw3DScene(void) {
    Stop3DScene();
    Start2DScene();
    renderer->PushMatrix();
    renderer->Translate(0.5, 0.5, 0);
    renderer->Scale(1, -1, 1);
    glDepthFunc(GL_ALWAYS);
    glDisable(GL_CULL_FACE);
    SetViewport(::Viewport(m_sceneLeft, 0, m_sceneWidth, m_sceneHeight), false);
    m_renderTexture.m_handle = m_sceneBuffer.BufferHandle(0);
    m_viewportArea.Render(&m_renderTexture);
    renderer->PopMatrix();
}


void BasicRenderer::DrawScreen (bool bRotate) {
    if (m_screenIsValid) {
        m_screenIsValid = false;
        Stop2DScene();
        m_renderTexture.m_handle = m_screenBuffer.BufferHandle(0);
        Translate(0.5, 0.5, 0);
        if (bRotate)
            Rotate(90, 0, 0, 1);
        else
            Scale(1, -1, 1);
        glDepthFunc(GL_ALWAYS);
        glDisable(GL_CULL_FACE); // required for vertical flipping because that inverts the buffer's winding
        glClear(GL_COLOR_BUFFER_BIT);
        m_viewportArea.Render(&m_renderTexture);
    }
}


void BasicRenderer::SetViewport(bool isFBO) {
    SetViewport(m_viewport, isFBO);
}


void BasicRenderer::SetViewport(::Viewport viewport, bool flipVertically) {
    m_viewport = viewport;
#if 1 //USE_RTT
    if (flipVertically)
        glViewport(viewport.m_left, m_windowHeight - viewport.m_top - viewport.m_height, viewport.m_width, viewport.m_height);
    else
        glViewport(viewport.m_left, viewport.m_top, viewport.m_width, viewport.m_height);
#else
    glViewport(viewport.m_left, viewport.m_top, viewport.m_width, viewport.m_height);
#endif
}


void BasicRenderer::SaveDrawBuffer() {
#if USE_RTT
    m_drawBufferStack.Push(m_drawBufferInfo);
#endif
}


void BasicRenderer::SetDrawBuffers(FBO* fbo, ManagedArray<GLuint>* drawBuffers) {
#if USE_RTT
    if ((fbo != nullptr) && (m_drawBufferInfo.m_fbo != nullptr) && (fbo->m_handle == m_drawBufferInfo.m_fbo->m_handle))
        m_drawBufferInfo.m_drawBuffers = drawBuffers;
    else {
        SaveDrawBuffer();
        m_drawBufferInfo = DrawBufferInfo(fbo, drawBuffers);
    }
    ClearGLError();
    glDrawBuffers(m_drawBufferInfo.m_drawBuffers->Length(), m_drawBufferInfo.m_drawBuffers->Data());
#else
    glDrawBuffer(drawBuffer);
#endif
    CheckGLError("setting draw buffer");
}


void BasicRenderer::RestoreDrawBuffer(void) {
#if USE_RTT
    m_drawBufferStack.Pop(m_drawBufferInfo);
    glBindTexture(GL_TEXTURE_2D, 0);
    if (m_drawBufferInfo.m_fbo != nullptr)
        m_drawBufferInfo.m_fbo->Reenable();
    else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDrawBuffers(m_drawBufferInfo.m_drawBuffers->Length(), m_drawBufferInfo.m_drawBuffers->Data());
    }
#else
    glDrawBuffer(GL_BACK);
#endif
}


void BasicRenderer::Fill(const RGBColor& color, float alpha, float scale) {
    renderer->PushMatrix();
    renderer->Translate(0.5, 0.5, 0.0);
    renderer->Scale(scale, scale, 1);
    m_viewportArea.Fill(color, alpha);
    renderer->PopMatrix();
}


void BasicRenderer::ClearGLError(void) {
    while (glGetError() != GL_NO_ERROR)
        ;
}


bool BasicRenderer::CheckGLError (const char* operation) {
    GLenum glError = glGetError ();
    if (glError) {
        fprintf (stderr, "OpenGL Error %d (%s)\n", glError, operation);
        return false;
    }
    return true;
}

BasicRenderer* renderer = nullptr;

// =================================================================================================
