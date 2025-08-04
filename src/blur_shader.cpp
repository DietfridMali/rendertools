
#include "array.hpp"
#include "string.hpp"
#include "basic_shadercode.h"

// =================================================================================================

const ShaderSource& BoxBlurShader() {
    static const ShaderSource boxBlurShader(
        "boxblur",
        StandardVS(),
        R"(
            //#version 140
            //#extension GL_ARB_explicit_attrib_location : enable
            #version 330
            uniform sampler2D source;
            uniform float FXAA_SPAN_MAX = 16.0;
            uniform float FXAA_REDUCE_MIN = 1.0 / 128.0;
            uniform float FXAA_REDUCE_MUL = 1.0 / 8.0;
            in vec2 fragTexCoord;
            out vec4 fragColor;
            vec3 FxaaPixelShader(vec2 pos, sampler2D tex, vec2 texelSize) {
                vec3 rgbNW = textureOffset(tex, pos, ivec2(-1, -1)).xyz;
                vec3 rgbNE = textureOffset(tex, pos, ivec2(1, -1)).xyz;
                vec3 rgbSW = textureOffset(tex, pos, ivec2(-1, 1)).xyz;
                vec3 rgbSE = textureOffset(tex, pos, ivec2(1, 1)).xyz;
                vec3 rgbM = textureLod(tex, pos, 0.0).xyz;
                const vec3 luma = vec3(0.299, 0.587, 0.114);
                float lumaNW = dot(rgbNW, luma);
                float lumaNE = dot(rgbNE, luma);
                float lumaSW = dot(rgbSW, luma);
                float lumaSE = dot(rgbSE, luma);
                float lumaM = dot(rgbM, luma);
                float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
                float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
                vec2 dir = vec2(-((lumaNW + lumaNE) - (lumaSW + lumaSE)), ((lumaNW + lumaSW) - (lumaNE + lumaSE)));
                float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
                float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
                dir = min(vec2(FXAA_SPAN_MAX), max(vec2(-FXAA_SPAN_MAX), dir * rcpDirMin)) * texelSize;
                vec3 rgbA = 0.5 * (textureLod(tex, pos + dir * (1.0 / 3.0 - 0.5), 0.0).xyz + textureLod(tex, pos + dir * (2.0 / 3.0 - 0.5), 0.0).xyz);
                vec3 rgbB = rgbA * 0.5 + 0.25 * (textureLod(tex, pos + dir * -0.5, 0.0).xyz + textureLod(tex, pos + dir * 0.5, 0.0).xyz);
                float lumaB = dot(rgbB, luma);
                return (lumaB < lumaMin || lumaB > lumaMax) ? rgbA : rgbB;
                }
            void main() {
                vec2 texelSize = 1.0 / vec2(textureSize(source, 0));
                vec3 color = FxaaPixelShader(fragTexCoord, source, texelSize);
                fragColor = vec4(color, texture(source, fragTexCoord).a);
                }
            )"
    );
    return boxBlurShader;
}

const ShaderSource& FxaaShader() {
    static const ShaderSource fxaaShader(
        "fxaa",
        StandardVS(),
        R"(
        //#version 140
        //#extension GL_ARB_explicit_attrib_location : enable
        #version 330
        uniform sampler2D source;
        uniform float FXAA_SPAN_MAX = 16.0;
        uniform float FXAA_REDUCE_MIN = 1.0 / 128.0;
        uniform float FXAA_REDUCE_MUL = 1.0 / 8.0;
        in vec2 fragTexCoord;
        out vec4 fragColor;
        vec3 FxaaPixelShader(vec2 pos, sampler2D tex, vec2 texelSize) {
            vec3 rgbNW = textureOffset(tex, pos, ivec2(-1, -1)).xyz;
            vec3 rgbNE = textureOffset(tex, pos, ivec2(1, -1)).xyz;
            vec3 rgbSW = textureOffset(tex, pos, ivec2(-1, 1)).xyz;
            vec3 rgbSE = textureOffset(tex, pos, ivec2(1, 1)).xyz;
            vec3 rgbM = textureLod(tex, pos, 0.0).xyz;
            const vec3 luma = vec3(0.299, 0.587, 0.114);
            float lumaNW = dot(rgbNW, luma);
            float lumaNE = dot(rgbNE, luma);
            float lumaSW = dot(rgbSW, luma);
            float lumaSE = dot(rgbSE, luma);
            float lumaM = dot(rgbM, luma);
            float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
            float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
            vec2 dir = vec2(-((lumaNW + lumaNE) - (lumaSW + lumaSE)), ((lumaNW + lumaSW) - (lumaNE + lumaSE)));
            float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
            float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
            dir = min(vec2(FXAA_SPAN_MAX), max(vec2(-FXAA_SPAN_MAX), dir * rcpDirMin)) * texelSize;
            vec3 rgbA = 0.5 * (textureLod(tex, pos + dir * (1.0 / 3.0 - 0.5), 0.0).xyz + textureLod(tex, pos + dir * (2.0 / 3.0 - 0.5), 0.0).xyz);
            vec3 rgbB = rgbA * 0.5 + 0.25 * (textureLod(tex, pos + dir * -0.5, 0.0).xyz + textureLod(tex, pos + dir * 0.5, 0.0).xyz);
            float lumaB = dot(rgbB, luma);
            return (lumaB < lumaMin || lumaB > lumaMax) ? rgbA : rgbB;
        }
        void main() {
            vec2 texelSize = 1.0 / vec2(textureSize(source, 0));
            vec3 color = FxaaPixelShader(fragTexCoord, source, texelSize);
            fragColor = vec4(color, texture(source, fragTexCoord).a);
        }
        )"
    );
    return fxaaShader;
}

const ShaderSource& GaussBlurShader() {
    static const ShaderSource gaussBlurShader(
        "gaussblur",
        StandardVS(),
        R"(
        //#version 140
        //#extension GL_ARB_explicit_attrib_location : enable
        #version 330
        uniform sampler2D source;
        uniform float direction;
        in vec2 fragTexCoord;
        out vec4 fragColor;
        uniform int radius; 
        uniform float coeffs[33];
        void main() {
           vec2 texelSize = 1.0 / vec2(textureSize(source, 0));
           vec2 offset = vec2 (1 - direction, direction);
    	    vec4 sum = vec4(0.0);
           int n = 2 * radius + 1;
    	    for (int i = 0; i < n; ++i)	{
    		    vec2 coord = fragTexCoord + offset * float(i - radius) * texelSize;
    		    sum += vec4 (coeffs[i] * texture(source, coord));
    	    }
    	    fragColor = sum;
        }
        )"
    );
    return gaussBlurShader;
}

// =================================================================================================
