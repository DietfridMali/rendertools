#include "rendermatrices.h"

#define DEBUG_MATRICES 1

#if DEBUG_MATRICES
bool RenderMatrices::m_legacyMode = false;
#else
bool RenderMatrices::m_legacyMode = false;
#endif

List<Matrix4f> RenderMatrices::matrixStack;

// =================================================================================================

void RenderMatrices::CreateMatrices(int windowWidth, int windowHeight, float aspectRatio, float fov) {
    m_renderMatrices[mtModelView] = Matrix4f::IDENTITY;
    m_renderMatrices[mtProjection2D] = m_projection.ComputeOrthoProjection(0.0f, float(windowWidth), 0.0f, float(windowHeight), -1.0f, 1.0f);
    m_renderMatrices[mtProjection2D].AsArray();
    m_renderMatrices[mtProjection3D] = m_projection.Create(aspectRatio, fov, true);
    m_renderMatrices[mtProjection3D].AsArray();
}


void RenderMatrices::SetupTransformation(void) {
    if (DEBUG_MATRICES or m_legacyMode)  {
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(m_renderMatrices[mtProjection3D].AsArray()); // already column major
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }
#if !DEBUG_MATRICES
    else 
#endif
    {
        m_renderMatrices[mtModelView] = Matrix4f::IDENTITY;
        m_renderMatrices[mtProjection] = m_renderMatrices[mtProjection3D];
    }
}


void RenderMatrices::ResetTransformation(void) {
    if (DEBUG_MATRICES or m_legacyMode)  {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }
#if !DEBUG_MATRICES
    else
#endif
    {
        m_renderMatrices[mtModelView] = Matrix4f::IDENTITY;
        m_renderMatrices[mtProjection] = m_renderMatrices[mtProjection2D];
    }
}


bool RenderMatrices::CheckModelView(void) {
#if DEBUG_MATRICES
    float glData[16], mData[16];
    Shader::GetFloatData(GL_MODELVIEW_MATRIX, 16, glData);
    memcpy(mData, ModelView().AsArray(), sizeof(mData));
    for (int i = 0; i < 16; i++) {
        if (abs(glData[i] - mData[i]) > 0.001f) {
            return false;
        }
    }
#endif
    return true;
}


bool RenderMatrices::CheckProjection(void) {
#if DEBUG_MATRICES
    float glData[16], mData[16];
    Shader::GetFloatData(GL_PROJECTION_MATRIX, 16, glData);
    memcpy(mData, Projection().AsArray(), sizeof(mData));
    for (int i = 0; i < 16; i++) {
        if (abs(glData[i] - mData[i]) > 0.001f) {
            return false;
        }
    }
#endif
    return true;
}


Matrix4f& RenderMatrices::Scale(float xScale, float yScale, float zScale, const char* caller) {
    //fprintf(stderr, "   Scale(%1.2f, %1.2f, %1.2f)\n", xScale, yScale, zScale);
#if DEBUG_MATRICES
    float glData[16];
    Shader::GetFloatData(GL_MODELVIEW_MATRIX, 16, glData);
    Matrix4f m;
    m = ModelView().IsColMajor() ? ModelView() : ModelView().Transpose(m, 3);
    CheckModelView();
#endif
    if (DEBUG_MATRICES or m_legacyMode)
        glScalef(xScale, yScale, zScale);
#if !DEBUG_MATRICES
    else
#endif
        ModelView().Scale(xScale, yScale, zScale);
#if DEBUG_MATRICES
    CheckModelView();
#endif
    return ModelView();
}


Matrix4f& RenderMatrices::Translate(float xTranslate, float yTranslate, float zTranslate, const char* caller) {
    //fprintf(stderr, "   Translate(%1.2f, %1.2f, %1.2f)\n", xTranslate, yTranslate, zTranslate);
#if DEBUG_MATRICES
    float glData[16];
    Shader::GetFloatData(GL_MODELVIEW_MATRIX, 16, glData);
    Matrix4f m;
    m = ModelView().IsColMajor() ? ModelView() : ModelView().Transpose(m, 3);
#endif
    if (DEBUG_MATRICES or m_legacyMode) {
        glTranslatef(xTranslate, yTranslate, zTranslate);
    }
#if !DEBUG_MATRICES
    else
#endif
    {
#if USE_GLM
        ModelView().Translate(xTranslate, yTranslate, zTranslate);
#else
#   if 1
        ModelView().Translate(xTranslate, yTranslate, zTranslate);
#   else
        Matrix4f t = Matrix4f::Translation(xTranslate, yTranslate, zTranslate, ModelView().IsColMajor());
        ModelView() = t * ModelView();
#   endif
#endif
    }
#if DEBUG_MATRICES
    CheckModelView();
#endif
    return ModelView();
}


Matrix4f& RenderMatrices::Rotate(float angle, float xScale, float yScale, float zScale, const char* caller) {
#if DEBUG_MATRICES
    float glData[16];
    Shader::GetFloatData(GL_MODELVIEW_MATRIX, 16, glData);
    Matrix4f m = ModelView();
    CheckModelView();
#endif
    if (DEBUG_MATRICES or m_legacyMode) 
        glRotatef(angle, xScale, yScale, zScale);
#if !DEBUG_MATRICES
    else
#endif
        ModelView().Rotate(angle, xScale, yScale, zScale);
#if DEBUG_MATRICES
    CheckModelView();
#endif
    return ModelView();
}


Matrix4f& RenderMatrices::Rotate(Matrix4f& r) {
#if DEBUG_MATRICES
    float glData[16];
    Shader::GetFloatData(GL_MODELVIEW_MATRIX, 16, glData);
    Matrix4f m = ModelView();
    CheckModelView();
#endif
    if (DEBUG_MATRICES or m_legacyMode)
        glMultMatrixf(r.AsArray());
#if !DEBUG_MATRICES
    else
#endif
        ModelView().Rotate(r);
#if DEBUG_MATRICES
    CheckModelView();
#endif
    return ModelView();
}


Matrix4f& RenderMatrices::Rotate(Vector3f angles) {
#if USE_GLM
    Matrix4f r = Matrix4f::Rotation(angles);
#else
    Matrix4f r = Matrix4f::Rotation(angles, ModelView().IsColMajor());
#endif
    return Rotate(r);
}


void RenderMatrices::PushMatrix(eMatrixType matrixType) {
    //fprintf(stderr, "PushMatrix\n");
    if (DEBUG_MATRICES or m_legacyMode) {
        glMatrixMode((matrixType == mtModelView) ? GL_MODELVIEW : GL_PROJECTION);
        glPushMatrix();
    }
#if !DEBUG_MATRICES
    else
#endif
    {
        PushMatrix(m_renderMatrices[matrixType]);
    }
}


void RenderMatrices::PopMatrix(eMatrixType matrixType) {
    //fprintf(stderr, "PopMatrix\n");
    if (DEBUG_MATRICES or m_legacyMode) {
        glMatrixMode((matrixType == mtModelView) ? GL_MODELVIEW : GL_PROJECTION);
        glPopMatrix();
    }
#if !DEBUG_MATRICES
    else
#endif
    {
        PopMatrix(m_renderMatrices[matrixType]);
#ifdef _DEBUG
        m_renderMatrices[matrixType].AsArray();
#endif
    }
}

// =================================================================================================
