
#include <utility>
#include "shader.h"
#include "base_renderer.h"

// =================================================================================================
// Some basic shader handling: Compiling, enabling, setting shader variables

Dictionary<String, UniformHandle*> Shader::uniforms;

String Shader::GetInfoLog (GLuint handle, bool isProgram)
{
    int logLength = 0;
    int charsWritten = 0;

    if (isProgram)
        glGetProgramiv (handle, GL_INFO_LOG_LENGTH, &logLength);
    else
        glGetShaderiv (handle, GL_INFO_LOG_LENGTH, &logLength);

    if (not logLength)
        return String ("no log found.\n");
    String infoLog;
    infoLog.Resize(logLength);
    if (isProgram)
        glGetProgramInfoLog (handle, logLength, &charsWritten,infoLog.Data());
    else
        glGetShaderInfoLog (handle, logLength, &charsWritten, infoLog.Data());
    fprintf (stderr, "Shader info: %s\n\n", static_cast<char*>(infoLog));
    return infoLog;
    }


GLuint Shader::Compile(const char* code, GLuint type) {
    GLuint handle = glCreateShader(type);
    glShaderSource(handle, 1, (GLchar**)&code, nullptr);
    glCompileShader(handle);
    GLint isCompiled;
    glGetShaderiv (handle, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_TRUE)
        return handle;
    String shaderLog = GetInfoLog (handle);
    char buffer [10000];
    GLsizei bufLen;
    glGetShaderSource (handle, sizeof (buffer), &bufLen, buffer);
    fprintf(stderr, "\n***** compiler error in %s shader: *****\n\n", (char*)m_name);
    fprintf (stderr, "\nshader source:\n%s\n\n", buffer);
    glDeleteShader(handle);
    return 0;
}


GLuint Shader::Link(GLuint vsHandle, GLuint fsHandle) {
    if (not vsHandle or not fsHandle)
        return 0;
    GLuint handle = glCreateProgram();
    if (not handle)
        return 0;
    glAttachShader(handle, vsHandle);
    glAttachShader(handle, fsHandle);
    glLinkProgram(handle);
    GLint isLinked = 0;
    glGetProgramiv(handle, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_TRUE) {
        glDetachShader(handle, vsHandle);
        glDetachShader(handle, fsHandle);
        return handle;
    }
    String shaderLog = GetInfoLog (handle, true);
    char buffer [10000];
    GLsizei bufLen;
    fprintf(stderr, "\n***** linker error in %s shader: *****\n\n", (char*)m_name);
    glGetShaderSource (vsHandle, sizeof (buffer), &bufLen, buffer);
    fprintf (stderr, "\nVertex shader:\n%s\n\n", buffer);
    glGetShaderSource (fsHandle, sizeof (buffer), &bufLen, buffer);
    fprintf (stderr, "\nFragment shader:\n%s\n\n", buffer);
    glDeleteShader(vsHandle);
    glDeleteShader(fsHandle);
    glDeleteProgram(handle);
    return 0;
}


void Shader::UpdateMatrices(void) {
    float glData[16];
    if (RenderMatrices::m_legacyMode) {
        SetMatrix4f("mModelView", GetFloatData(GL_MODELVIEW_MATRIX, 16, glData));
        SetMatrix4f("mProjection", GetFloatData(GL_PROJECTION_MATRIX, 16, glData));
    }
    else {
        // both matrices must be column major
        SetMatrix4f("mModelView", baseRenderer.ModelView().AsArray(), false);
        SetMatrix4f("mProjection", baseRenderer.Projection().AsArray(), false);
#if 0
        SetMatrix4f("mBaseModelView", baseRenderer.ModelView().AsArray(), false);
#endif
    }
#if 0
    baseRenderer.CheckModelView();
    baseRenderer.CheckProjection();
    float glmData[16];
    memcpy(glmData, baseRenderer.Projection().AsArray(), sizeof(glmData));
#endif
}

// =================================================================================================
