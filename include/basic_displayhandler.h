#pragma once

#include <stdlib.h>
#include <math.h>
#include <functional>
#include "string.hpp"

// =================================================================================================
// basic renderer class. Initializes display and OpenGL and sets up projections and view matrix

class BasicDisplayHandler 
{

public:
    int             m_width;
    int             m_height;
    int             m_maxWidth;
    int             m_maxHeight;
    bool            m_fullscreen;
    bool            m_vSync;
    bool            m_isLandscape;
    float           m_aspectRatio;
    SDL_Window*     m_window;
    SDL_GLContext   m_context;

    BasicDisplayHandler(String windowTitle = "", int width = 1920, int height = 1080, bool fullscreen = true, bool vSync = true);

    virtual void ComputeDimensions(int width, int height, bool fullscreen);

    ~BasicDisplayHandler();

    void SetupDisplay(String windowTitle);

    void Update(void);

    inline int GetWidth(void) {
        return m_width;
    }

    inline int GetHeight(void) {
        return m_height;
    }

    inline float GetAspectRatio(void) {
        return m_aspectRatio;
    }
};

// =================================================================================================

