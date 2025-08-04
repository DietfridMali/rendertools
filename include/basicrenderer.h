#pragma once
#include <utility>

#include <stdlib.h>
#include <math.h>
#include "array.hpp"
#include "projection.h"
#include "matrix.hpp"
//#include "quad.h"
#include "viewport.h"
#include "fbo.h"
//#include "texture.h"
//#include "player.h"
#include "rendermatrices.h"

#define USE_RTT 1

// =================================================================================================

#if USE_RTT

class DrawBufferInfo {
public:
    FBO*            m_fbo;
    ManagedArray<GLuint>*  m_drawBuffers;

public:
    DrawBufferInfo(FBO* fbo = nullptr, ManagedArray<GLuint>* drawBuffers = nullptr) {
        m_fbo = fbo;
        m_drawBuffers = drawBuffers;
    }
};

#endif

// =================================================================================================
// basic renderer class. Initializes display and OpenGL and sets up projections and view matrix

class BasicRenderer 
    : public RenderMatrices
{

    public:
#if USE_RTT
        FBO                     m_screenBuffer;
        FBO                     m_sceneBuffer;
        FBO*                    m_activeBuffer;
        ManagedArray<GLuint>    m_drawBuffers;
        DrawBufferInfo          m_drawBufferInfo;
        List<DrawBufferInfo>    m_drawBufferStack;
#endif
        Viewport                m_viewport;
        Quad                    m_viewportArea;
        Texture                 m_renderTexture;

    protected:
        int                     m_windowWidth;
        int                     m_windowHeight;
        int                     m_sceneWidth;
        int                     m_sceneHeight;
        int                     m_sceneLeft;
        float                   m_aspectRatio;
        float                   m_maxDistance;
        bool                    m_screenIsValid;

    public:
        BasicRenderer(int width = 1920, int height = 1080); // , Viewer* viewer = nullptr);

        void Create(void);
            
        void SetupOpenGL (void);

        virtual void Start3DScene(void);

        virtual void Stop3DScene(void);

        virtual void Start2DScene(void);

        virtual void Stop2DScene(void);

        virtual void Draw3DScene(void);
            
        virtual void DrawScreen(bool bRotate);

        virtual void EnableCamera(void) { }

        void DisableCamera(void) { }

        inline FBO& SceneBuffer(void) {
            return m_sceneBuffer;
        }

        inline FBO& ScreenBuffer(void) {
            return m_screenBuffer;
        }

        inline void SetActiveBuffer(FBO* buffer, bool clearBuffer = false) {
            if (m_activeBuffer != buffer) {
                if (m_activeBuffer)
                    m_activeBuffer->Disable();
                m_activeBuffer = buffer;
            }
            if (!m_activeBuffer->IsEnabled())
                m_activeBuffer->Enable(0, clearBuffer);
        }

        inline int WindowWidth(void) {
            return m_windowWidth;
        }

        inline int WindowHeight(void) {
            return m_windowHeight;
        }

        inline int SceneWidth(void) {
            return m_sceneWidth;
        }

        inline int SceneHeight(void) {
            return m_sceneHeight;
        }

        inline float AspectRatio(void) {
            return m_aspectRatio;
        }

        typedef struct {
            int width, height;
        } tViewport;

        Viewport& Viewport(void) {
            return m_viewport;
        }

        void SetViewport(bool isFBO = false);

        void SetViewport(::Viewport viewport, bool flipVertically = false);

        inline ManagedArray<GLuint>* DrawBuffer(void) {
            return m_drawBufferInfo.m_drawBuffers;
        }

        void SaveDrawBuffer();

        void SetDrawBuffers(FBO* fbo, ManagedArray<GLuint>* drawBuffers);

        void RestoreDrawBuffer(void);

        void ResetDrawBuffers(FBO* activeBuffer, bool clearBuffer = true);

        void Fill(const RGBColor& color, float alpha, float scale);

        inline void Fill(RGBColor&& color, float alpha, float scale) {
            Fill(static_cast<const RGBColor&>(color), alpha, scale);
        }

        void ClearGLError(void);

        bool CheckGLError (const char* operation = "");
};

// =================================================================================================
