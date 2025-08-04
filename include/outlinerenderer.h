#pragma once

#include "fbo.h"

// =================================================================================================

class OutlineRenderer {
public:
    typedef struct {
        String  method;
        int     strength;
    } tAAMethod;

    void AntiAlias(FBO* fbo, tAAMethod aaMethod);

    void RenderOutline(FBO* fbo, float outlineWidth, RGBAColor color = Vector4f{0, 0, 0, 1}, tAAMethod aaMethod = { "boxblur", 0 });
};

// =================================================================================================


