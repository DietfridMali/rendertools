#pragma once

#include "vector.hpp"
#include "matrix.hpp"
#include "glew.h"
#include "shader.h"
#include "projection.h"

// =================================================================================================

#define FIXED_RENDER_PIPELINE 1

class RenderMatrices {
public:
    typedef enum {
        mtModelView,
        mtProjection,
        mtProjection2D,
        mtProjection3D,
        mtCount
    } eMatrixType;

    Projection  m_projection;
    Matrix4f    m_renderMatrices[mtCount]; // matrices are row major - let OpenGL transpose them when passing them with glUniformMatrix4fv
    Matrix4f    m_glProjection[3];
    Matrix4f    m_glModelView[3];

    static List<Matrix4f> matrixStack;
    static bool           m_legacyMode;

    RenderMatrices() { }


    void CreateMatrices(int windowWidth, int windowHeight, float aspectRatio);


    inline float ZNear(void) {
        return m_projection.m_zNear;
    }


    inline float ZFar(void) {
        return m_projection.m_zFar;
    }


    inline Matrix4f& ModelView(void) {
        return m_renderMatrices[mtModelView];
    }


    inline Matrix4f& Projection(void) {
        return m_renderMatrices[mtProjection];
    }


    inline GLfloat* ProjectionMatrix(void) {
        return (GLfloat*)m_renderMatrices[mtProjection].AsArray();
    }

    // setup 3D transformation and projection
    void SetupTransformation(void);


    void ResetTransformation(void);


    template<typename T>
    void SetMatrix(T&& m, eMatrixType matrixType = mtModelView) {
        if (matrixType == mtModelView) {
            ModelView() = std::forward<T>(m);
            glMatrixMode(GL_MODELVIEW);
            glLoadMatrixf(std::forward<T>(m).Transpose().AsArray());
        }
        else {
            Projection() = std::forward<T>(m);
            glMatrixMode(GL_PROJECTION);
            glLoadMatrixf(std::forward<T>(m).Transpose().AsArray());
        }
    }


    void PushMatrix(eMatrixType matrixType = mtModelView);


    void PopMatrix(eMatrixType matrixType = mtModelView);


    bool CheckModelView(void);


    bool CheckProjection(void);


    Matrix4f& Scale(float xScale, float yScale, float zScale, const char* caller = "");


    Matrix4f& Translate(float xTranslate, float yTranslate, float zTranslate, const char* caller = "");


    Matrix4f& Rotate(float angle, float xScale, float yScale, float zScale, const char* caller = "");


    Matrix4f& Rotate(Matrix4f& r);


    Matrix4f& Rotate(Vector3f angles);


    inline void Translate(Vector3f v) {
        Translate(v.X(), v.Y(), v.Z());
    }


    inline Matrix4f& Scale(float scale) {
        return Scale(scale, scale, scale);
    }


    inline Matrix4f& Scale(Vector3f scale) {
        return Scale(scale.X(), scale.Y(), scale.Z());
    }

    
    inline Vector3f Project(Vector3f v) {
        return static_cast<Vector3f>(Projection() * (ModelView() * static_cast<Vector4f>(v)));
    }


    static void PushMatrix(Matrix4f& m) {
        matrixStack.Append(m);
    }


    static Matrix4f& PopMatrix(Matrix4f& m) {
        matrixStack.Pop(m);
        return m;
    }
};

// =================================================================================================
