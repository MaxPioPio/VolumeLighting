#include "primitives.hpp"

#include <QDebug>
#include <QOpenGLFunctions_4_0_Core>

#include "glutils.hpp"



PrimitiveUtils::PrimitiveUtils() {    
    cubeVbo = GL_INVALID_VALUE;
    sphereVbo = GL_INVALID_VALUE;
    cylinderVbo = GL_INVALID_VALUE;
    coneVbo = GL_INVALID_VALUE;
    torusVbo = GL_INVALID_VALUE;
    planeVbo = GL_INVALID_VALUE;

    cubeIbo = GL_INVALID_VALUE;
    sphereIbo = GL_INVALID_VALUE;
    cylinderIbo = GL_INVALID_VALUE;
    coneIbo = GL_INVALID_VALUE;
    torusIbo = GL_INVALID_VALUE;
    planeIbo = GL_INVALID_VALUE;

    cubeVao = GL_INVALID_VALUE;
    sphereVao = GL_INVALID_VALUE;
    cylinderVao = GL_INVALID_VALUE;
    coneVao = GL_INVALID_VALUE;
    torusVao = GL_INVALID_VALUE;
    planeVao = GL_INVALID_VALUE;

    initialized = false;

    init();
}


void PrimitiveUtils::init() {
    if(initialized)
        return;

    QOpenGLFunctions_4_0_Core* glf = GLUtils::glFunc();
    if(!glf) {
        qInfo() << "OpenGL Functions 4.0 Core not initialized!";
        return;
    }

    // Create VBOs
    glf->glGenBuffers(1, &cubeVbo);
    glf->glGenBuffers(1, &sphereVbo);
    glf->glGenBuffers(1, &cylinderVbo);
    glf->glGenBuffers(1, &coneVbo);
    glf->glGenBuffers(1, &torusVbo);
    glf->glGenBuffers(1, &planeVbo);
    // Create IBOs
    glf->glGenBuffers(1, &cubeIbo);
    glf->glGenBuffers(1, &sphereIbo);
    glf->glGenBuffers(1, &cylinderIbo);
    glf->glGenBuffers(1, &coneIbo);
    glf->glGenBuffers(1, &torusIbo);
    glf->glGenBuffers(1, &planeIbo);
    // Create VAOs
    glf->glGenVertexArrays(1, &cubeVao);
    glf->glGenVertexArrays(1, &sphereVao);
    glf->glGenVertexArrays(1, &cylinderVao);
    glf->glGenVertexArrays(1, &coneVao);
    glf->glGenVertexArrays(1, &torusVao);
    glf->glGenVertexArrays(1, &planeVao);

    //---------------------------- setup the buffers -------------------------------
    // Cube
    glf->glBindVertexArray(cubeVao);
        // VBO
    glf->glBindBuffer(GL_ARRAY_BUFFER, cubeVbo);
    glf->glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVx), cubeVx, GL_STATIC_DRAW);
    glf->glEnableVertexAttribArray(0);
    glf->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), 0); // vertex
    glf->glEnableVertexAttribArray(1);
    glf->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (GLvoid*) (3*sizeof(float))); // normal

        // IBO
    glf->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeIbo);
    glf->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIx), cubeIx, GL_STATIC_DRAW);

    // [...]
    // Plane XY
    glf->glBindVertexArray(planeVao);
        // VBO
    glf->glBindBuffer(GL_ARRAY_BUFFER, planeVbo);
    glf->glBufferData(GL_ARRAY_BUFFER, sizeof(planeVx), planeVx, GL_STATIC_DRAW);
    glf->glEnableVertexAttribArray(0);
    glf->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), 0); // vertex
    glf->glEnableVertexAttribArray(1);
    glf->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (GLvoid*) (3*sizeof(float))); // normal
        // IBO
    glf->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planeIbo);
    glf->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(planeIx), planeIx, GL_STATIC_DRAW);

qInfo() << "after init : " << GLUtils::glError();

    // protect the last vertex array from later changes
    glf->glBindVertexArray(0);

    // initialization complete!
    initialized = true;
}

void PrimitiveUtils::cleanup() {
    if(!initialized)
        return;

    initialized = false;
    QOpenGLFunctions_4_0_Core* glf = GLUtils::glFunc();

    // Delete VBOs
    glf->glDeleteBuffers(1, &cubeVbo);
    glf->glDeleteBuffers(1, &sphereVbo);
    glf->glDeleteBuffers(1, &cylinderVbo);
    glf->glDeleteBuffers(1, &coneVbo);
    glf->glDeleteBuffers(1, &torusVbo);
    glf->glDeleteBuffers(1, &planeVbo);
    // Delete IBOs
    glf->glDeleteBuffers(1, &cubeIbo);
    glf->glDeleteBuffers(1, &sphereIbo);
    glf->glDeleteBuffers(1, &cylinderIbo);
    glf->glDeleteBuffers(1, &coneIbo);
    glf->glDeleteBuffers(1, &torusIbo);
    glf->glDeleteBuffers(1, &planeIbo);
    // Delete VAOs
    glf->glDeleteVertexArrays(1, &cubeVao);
    glf->glDeleteVertexArrays(1, &sphereVao);
    glf->glDeleteVertexArrays(1, &cylinderVao);
    glf->glDeleteVertexArrays(1, &coneVao);
    glf->glDeleteVertexArrays(1, &torusVao);
    glf->glDeleteVertexArrays(1, &planeVao);
}

void PrimitiveUtils::renderPrimitive(Prim3D type) {

    if(!initialized) {
        qWarning("PrimitiveUtils not initialized!");
        return;
    }

    switch(type) {
    case Prim3D::CUBE:
        renderCube();
        break;
    case Prim3D::SPHERE:
        renderSphere();
        break;
    case Prim3D::CYLINDER:
        renderCylinder();
        break;
    case Prim3D::CONE:
        renderCone();
        break;
    case Prim3D::TORUS:
        renderTorus();
        break;
    case Prim3D::PLANE:
        renderPlaneXZ();
        break;
    }
}


// RENDER METHODS ////////////////////////////////////////////////
void PrimitiveUtils::renderCube() {
    if(!initialized) {
        qWarning("PrimitiveUtils not initialized!");
        return;
    }
    QOpenGLFunctions_4_0_Core* glf = GLUtils::glFunc();

    glf->glBindVertexArray(cubeVao);
    glf->glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (GLvoid*)0);
}

void PrimitiveUtils::renderSphere() {
    if(!initialized) {
        qWarning("PrimitiveUtils not initialized!");
        return;
    }
    qWarning("Primtive not yet implemented!");
}

void PrimitiveUtils::renderCylinder() {
    if(!initialized) {
        qWarning("PrimitiveUtils not initialized!");
        return;
    }
    qWarning("Primtive not yet implemented!");
}

void PrimitiveUtils::renderCone() {
    if(!initialized) {
        qWarning("PrimitiveUtils not initialized!");
        return;
    }
    qWarning("Primtive not yet implemented!");
}

void PrimitiveUtils::renderTorus() {
    if(!initialized) {
        qWarning("PrimitiveUtils not initialized!");
        return;
    }
    qWarning("Primtive not yet implemented!");
}

void PrimitiveUtils::renderPlaneXY() {
    if(!initialized) {
        qWarning("PrimitiveUtils not initialized!");
        return;
    }
    QOpenGLFunctions_4_0_Core* glf = GLUtils::glFunc();
    glf->glBindVertexArray(planeVao);
    glf->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (GLvoid*)0);    // offset 0 = xy
}

void PrimitiveUtils::renderPlaneXZ() {
    if(!initialized) {
        qWarning("PrimitiveUtils not initialized!");
        return;
    }
    QOpenGLFunctions_4_0_Core* glf = GLUtils::glFunc();
    glf->glBindVertexArray(planeVao);
    glf->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (GLvoid*)(6*sizeof(ushort)));    // offset 6 = xz
}

void PrimitiveUtils::renderPlaneYZ() {
    if(!initialized) {
        qWarning("PrimitiveUtils not initialized!");
        return;
    }
    QOpenGLFunctions_4_0_Core* glf = GLUtils::glFunc();
    glf->glBindVertexArray(planeVao);
    glf->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (GLvoid*)(12*sizeof(ushort)));    // offset 12 = yz
}


// VERTEX/NORMAL AND INDEX ARRAYS ///////////////////////////////////////
float PrimitiveUtils::cubeVx[144] = {-0.5f, -0.5f, -0.5f, 0, -1, 0, // lower xz plane
                                    -0.5f, -0.5f, 0.5f, 0, -1, 0,  // 0 1 2 3
 /*   5---6     */                  0.5f, -0.5f, 0.5f, 0, -1, 0,
 /*  /|  /|     */                  0.5f, -0.5f, -0.5f, 0, -1, 0,
 /* 4---7 |     */                  -0.5f, 0.5f, -0.5f, 0, 1, 0, // upper xz plane
 /* | 1-|-2     */                  -0.5f, 0.5f, 0.5f, 0, 1, 0,  // 4 5 6 7
 /* |/  |/      */                  0.5f, 0.5f, 0.5f, 0, 1, 0,
 /* 0---3       */                  0.5f, 0.5f, -0.5f, 0, 1, 0,
                                    -0.5f, -0.5f, 0.5f, 0, 0, 1, // front xy plane
                                    0.5f, -0.5f, 0.5f, 0, 0, 1,  // 0 3 7 4
                                    0.5f, 0.5f, 0.5f, 0, 0, 1,
                                    -0.5f, 0.5f, 0.5f, 0, 0, 1,
                                    0.5f, -0.5f, -0.5f, 0, 0, -1,  // back xy plane
                                    -0.5f, -0.5f, -0.5f, 0, 0, -1, // 2 1 5 6
                                    -0.5f, 0.5f, -0.5f, 0, 0, -1,
                                    0.5f, 0.5f, -0.5f, 0, 0, -1,
                                    0.5f, -0.5f, 0.5f, 1, 0, 0,  // right yz plane
                                    0.5f, -0.5f, -0.5f, 1, 0, 0,   // 3 2 6 7
                                    0.5f, 0.5f, -0.5f, 1, 0, 0,
                                    0.5f, 0.5f, 0.5f, 1, 0, 0,
                                    -0.5f, -0.5f, -0.5f, -1, 0, 0,    // left yz plane
                                    -0.5f, -0.5f, 0.5f, -1, 0, 0,  // 1 0 4 5
                                    -0.5f, 0.5f, 0.5f, -1, 0, 0,
                                    -0.5f, 0.5f, -0.5f, -1, 0, 0
                                    };
ushort PrimitiveUtils::cubeIx[36] = {0,3,2, 0,2,1,  // bottom
                                     4,6,7, 4,5,6,  // top
                                     8,9,10, 8,10,11, // front
                                     12,13,14, 12,14,15, // back
                                     16,17,18, 16,18,19, // right
                                     20,21,22, 20,22,23}; // left


float PrimitiveUtils::planeVx[72] = {// xy plane
                                     0.5f, -0.5f, 0.f, 0, 0, 1,
                                     -0.5f, -0.5f, 0.f, 0, 0, 1,
                                     -0.5f, 0.5f, 0.f, 0, 0, 1,
                                     0.5f, 0.5f, 0.f, 0, 0, 1,
                                     // xz plane
                                     -0.5f, 0.f, -0.5f, 0, 1, 0,
                                     -0.5f, 0.f, 0.5f, 0, 1, 0,
                                     0.5f, 0.f, 0.5f, 0, 1, 0,
                                     0.5f, 0.f, -0.5f, 0, 1, 0,
                                     // yz plane
                                     0.f, -0.5f, 0.5f, 1, 0, 0,
                                     0.f, -0.5f, -0.5f, 1, 0, 0,
                                     0.f, 0.5f, -0.5f, 1, 0, 0,
                                     0.f, 0.5f, 0.5f, 1, 0, 0};

ushort PrimitiveUtils::planeIx[18] = {0,3,2, 0,2,1,     // xy
                                      4,6,7, 4,5,6,     // xz
                                      8,9,10, 8,10,11}; // yz
