
#include "array.hpp"
#include "string.hpp"
#include "basic_shadercode.h"

// =================================================================================================

ShaderSource plainColorShader(
    "plainColor",
    standardVS,
    R"(
        //#version 140
        //#extension GL_ARB_explicit_attrib_location : enable
        #version 330
        uniform vec4 surfaceColor;
        uniform float maxDist;
        in vec3 fragPos;
        out vec4 fragColor;
    )"
    R"(
    void main() {
        fragColor = surfaceColor;
    }
    )"
    );

        // render a b/w mask with color applied.
ShaderSource plainTextureShader(
    "plainTexture",
    standardVS,
    R"(
        //#version 140
        //#extension GL_ARB_explicit_attrib_location : enable
        #version 330
        uniform sampler2D source;
        uniform vec4 surfaceColor;
        in vec3 fragPos;
        in vec2 fragTexCoord;

        layout(location = 0) out vec4 fragColor;
        
        void main() {
            vec4 texColor = texture (source, fragTexCoord);
            if (texColor.a == 0) discard;
            fragColor = vec4 (texColor.rgb * surfaceColor.rgb, texColor.a * surfaceColor.a);
            }
    )"
);

// =================================================================================================
