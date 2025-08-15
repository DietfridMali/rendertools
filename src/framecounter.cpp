
#include "framecounter.h"
#include "colordata.h"
#include "textrenderer.h"
#include "base_renderer.h"

// =================================================================================================

void BaseFrameCounter::Draw(bool update) {
    if (update)
        Update();
    baseRenderer.SetViewport(m_viewport, false);
    char szFPS[20];
    sprintf(szFPS, "%1.1f fps", GetFps()); // m_fps[0]);
    textRenderer.SetColor(m_color);
    textRenderer.Render(szFPS, TextRenderer::taLeft, false);
}

// -------------------------------------------------------------------------------------------------

void MovingFrameCounter::Update(void) {
    size_t t = SDL_GetTicks();
    if (m_renderStartTime > 0) {
        float dt = float(t - m_renderStartTime) * 0.001f;
        m_movingTotalTime -= m_movingFrameTimes[m_movingFrameIndex] + dt;
        m_movingFrameTimes[m_movingFrameIndex] = dt;
        m_movingFrameIndex = (m_movingFrameIndex + 1) % FrameWindowSize;
        if (m_movingFrameCount < FrameWindowSize)
            ++m_movingFrameCount;
    }
    m_renderStartTime = t;
}

// -------------------------------------------------------------------------------------------------

void LinearFrameCounter::Update(void) {
    ++m_frameCount[0];
    if (m_fpsTimer.HasExpired(0, true)) {
        m_fps[0] = float(m_frameCount[0]) / float((SDL_GetTicks() - m_renderStartTime) * 0.001f);
        m_fps[1] = float(m_frameCount[1]) / float(m_fpsTimer.LapTime() * 0.001f);
    }
    else
        ++m_frameCount[1];
}

// =================================================================================================
