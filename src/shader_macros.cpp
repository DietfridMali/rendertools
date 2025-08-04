
#include "array.hpp"
#include "string.hpp"
#include "basic_shadercode.h"

// =================================================================================================

String standardVS(
    R"(
        //#version 140
        //#extension GL_ARB_explicit_attrib_location : enable
        #version 330
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec2 texCoord;
        uniform mat4 mModelView;
        uniform mat4 mProjection;
        uniform float offset;
        out vec3 fragPos;
        out vec2 fragTexCoord;
        void main() {
            vec4 viewPos = mModelView * vec4 (position, 1.0);
            gl_Position = mProjection * viewPos; //.x + offset, position.y + offset, position.z, 1.0);
            fragTexCoord = texCoord;
            fragPos = viewPos.xyz;
            }
    )"
);

// =================================================================================================
