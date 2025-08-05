#pragma once
#include <utility>

#include <stdlib.h>
#include <math.h>
#include "singletonbase.hpp"
#include "array.hpp"
#include "matrix.hpp"
#include "projection.h"
#include "rendermatrices.h"
#include "viewport.h"
#include "fbo.h"

// =================================================================================================

class DrawBufferInfo {
public:
    FBO*                    m_fbo;
    ManagedArray<GLuint>*   m_drawBuffers;

public:
    DrawBufferInfo(FBO* fbo = nullptr, ManagedArray<GLuint>* drawBuffers = nullptr) {
        m_fbo = fbo;
        m_drawBuffers = drawBuffers;
    }
};

// =================================================================================================
// basic renderer class. Initializes display and OpenGL and sets up projections and view matrix

class BaseRenderer 
    : public RenderMatrices
    , public PolymorphSingleton

{
    public:
        FBO                     m_screenBuffer;
        FBO                     m_sceneBuffer;
        FBO*                    m_activeBuffer;
        ManagedArray<GLuint>    m_drawBuffers;
        DrawBufferInfo          m_drawBufferInfo;
        List<DrawBufferInfo>    m_drawBufferStack;
        Viewport                m_viewport;
        BaseQuad                m_viewportArea;
        Texture                 m_renderTexture;

    protected:
        int                     m_windowWidth;
        int                     m_windowHeight;
        int                     m_sceneWidth;
        int                     m_sceneHeight;
        int                     m_sceneLeft;
        float                   m_aspectRatio;
        bool                    m_screenIsAvailable;

    public:
        BaseRenderer()
            : m_activeBuffer(nullptr), m_windowWidth(0), m_windowHeight(0), m_sceneWidth(0), m_sceneHeight(0), m_sceneLeft(0), m_aspectRation(1.0f), m_screenIsAvailable(false)
        { }

        BaseRenderer& Instance(void) { return static_cast<BaseRenderer&>(SingletonInstance()); }

        void Init(int width, int height, float fov);

        void Create(int width = 1920, int height = 1080, float fov = 45);
            
        void SetupOpenGL (void);

        virtual bool Start3DScene(void);

        virtual bool Stop3DScene(void);

        virtual bool Start2DScene(void);

        virtual bool Stop2DScene(void);

        virtual void Draw3DScene(void);
            
        virtual void DrawScreen(bool bRotate);

        virtual void EnableCamera(void) { }

        virtual void DisableCamera(void) { }

        inline FBO& SceneBuffer(void) {
            return m_sceneBuffer;
        }

        inline FBO& ScreenBuffer(void) {
            return m_screenBuffer;
        }

        bool SetActiveBuffer(FBO* buffer, bool clearBuffer = false);

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

        void Fill(const RGBColor& color, float alpha, float scale = 1.0f);

        inline void Fill(RGBColor&& color, float alpha, float scale = 1.0f) {
            Fill(static_cast<const RGBColor&>(color), alpha, scale);
        }

        static void ClearGLError(void);

        static bool CheckGLError (const char* operation = "");
};

#define baseRenderer BaseRenderer::Instance()

// =================================================================================================
