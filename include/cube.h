#pragma once

#include "glew.h"
#include "vector.hpp"

class Cube {
public:
    static constexpr int vertexCount = 8;

    static Vector3f vertices[vertexCount];

    static constexpr int triangleIndexCount = 12;

    static GLuint triangleIndices[triangleIndexCount][3];

    static constexpr int quadIndexCount = 6;

    static GLuint quadIndices[quadIndexCount][4];
};