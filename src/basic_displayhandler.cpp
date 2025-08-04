
#include "std_defines.h"
#include "glew.h"
#include "SDL.h"
#include "basic_displayhandler.h"
#include <stdlib.h>
#include <math.h>
#include <algorithm>

// =================================================================================================

BasicDisplayHandler::BasicDisplayHandler(String windowTitle, int width, int height, bool fullscreen, bool vSync) {
    SDL_Rect rect;
    SDL_GetDisplayBounds(0, &rect);
    m_maxWidth = rect.w;
    m_maxHeight = rect.h;
    if (m_width * m_height == 0) {
        m_width = m_maxWidth;
        m_height = m_maxHeight;
        m_fullscreen = true;
    }
    else {
        m_width = std::min(width, m_maxWidth);
        m_height = std::min(height, m_maxHeight);
        m_fullscreen = fullscreen;
    }
    m_vSync = vSync;
    m_isLandscape = m_width > m_height;
    SetupDisplay(windowTitle);
}


BasicDisplayHandler::~BasicDisplayHandler() {
    SDL_GL_DeleteContext(m_context);
}


void BasicDisplayHandler::SetupDisplay(String windowTitle) {
    int screenType = SDL_WINDOW_OPENGL;
    if (m_fullscreen) {
        if ((m_width != m_maxWidth) or (m_height != m_maxHeight))
            screenType |= SDL_WINDOW_BORDERLESS;
        else
            screenType |= SDL_WINDOW_FULLSCREEN;
        m_fullscreen = true;
    }
    m_aspectRatio = float(m_width) / float(m_height);
#if 1
    m_window = SDL_CreateWindow(windowTitle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_width, m_height, screenType);
    if (not m_window) {
        fprintf(stderr, "Couldn't set screen mode (%d x %d)\n", m_width, m_height);
        exit(1);
    }
    m_context = SDL_GL_CreateContext(m_window);
    if (not m_context) {
        char msg[200];
        sprintf (msg, "Error '%s' when trying to create OpenGL context\n", SDL_GetError());
        throw std::runtime_error(msg);
    }
    SDL_GL_SetSwapInterval(m_vSync ? 1 : 0);
#endif
}


void BasicDisplayHandler::Update(void) {
    SDL_GL_SwapWindow(m_window);
}

// =================================================================================================
