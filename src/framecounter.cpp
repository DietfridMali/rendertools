#include "framecounter.h"
#include "colordata.h"
#include "textrenderer.h"
#include "base_renderer.h"

// =================================================================================================

void FrameCounter::Update(void) {
    ++m_frameCount[0];
    if (m_fpsTimer.HasExpired(0, true)) {
        m_fps[0] = float(m_frameCount[0]) / float((SDL_GetTicks() - m_renderStart) * 0.001f);
        m_fps[1] = float(m_frameCount[1]) / float(m_fpsTimer.LapTime() * 0.001f);
    }
    else
        ++m_frameCount[1];
}

void FrameCounter::Draw(bool update) {
    if (update)
        Update();
    baseRenderer.SetViewport(m_viewport, false);
    char szFPS[20];
    sprintf(szFPS, "%1.1f fps", m_fps[0]);
    textRenderer.SetColor(m_color);
    textRenderer.Render(szFPS, TextRenderer::taLeft, false);
}

// =================================================================================================
