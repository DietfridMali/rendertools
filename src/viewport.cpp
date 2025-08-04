#include <utility>
#include "glew.h"
#include "viewport.h"
#include "renderer.h"

// =================================================================================================

void Viewport::Fill(const RGBColor& color, float alpha, float scale) {
    SetViewport();
    renderer->Fill(color, alpha, scale);
}


void Viewport::SetViewport(void) {
    renderer->SetViewport(*this);
}


Viewport Viewport::Resize(int deltaLeft, int deltaTop, int deltaWidth, int deltaHeight) {
    return Viewport (m_left + deltaLeft, m_top + deltaTop, m_width + deltaWidth, m_height + deltaHeight);
}


void Viewport::SetResized(int deltaLeft, int deltaTop, int deltaWidth, int deltaHeight) {
    renderer->SetViewport (Resize (deltaLeft, deltaTop, deltaWidth, deltaHeight));
}

// =================================================================================================
