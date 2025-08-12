
#pragma once

#include "timer.h"
#include "viewport.h"

// =================================================================================================

class FrameCounter {
public:
    size_t                  m_frameCount[2]; // total frame count / frames during last second
    float                   m_fps[2];
    size_t                  m_renderStart;
    Timer                   m_fpsTimer;
    RGBAColor               m_color;
    Viewport                m_viewport;

    FrameCounter() {
        Reset();
    }

    ~FrameCounter() = default;

    void Setup(Viewport viewport, RGBAColor color = ColorData::Black) {
        m_viewport = viewport;
        m_color = color;
    }

    inline void Start(void) {
        if (m_frameCount == 0) {
            m_fpsTimer.Start();
            m_renderStart = m_fpsTimer.StartTime();
        }
    }

    inline void Reset(void) {
        m_frameCount[0] = m_frameCount[1] = 0;
        m_fps[0] = m_fps[1] = 0;
        m_renderStart = 0;
    }

    void Update(void);

    inline float GetFps(int i) {
        return m_fps[i];
    }

    void Draw(bool update);
};

// =================================================================================================
