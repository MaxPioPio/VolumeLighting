#pragma once

#include <QVector3D>
#include <QMatrix4x4>
#include <QtMath>
#include <QQuaternion>
#ifdef WIN32
    #include <windows.h>
#endif
#include <QOpenGLContext>
#include <GL/gl.h>

enum Prim3D {
    NONE = 0,
    CUBE = 1,
    SPHERE = 2,
    CYLINDER = 3,
    CONE = 4,
    TORUS = 5,
    PLANE = 6
};

class PrimitiveUtils
{

public:
    PrimitiveUtils();
    void init();
    void cleanup();
    void renderPrimitive(Prim3D type);

    void renderCube();
    void renderSphere();
    void renderCylinder();
    void renderCone();
    void renderTorus();
    void renderPlaneXY();
    void renderPlaneXZ();
    void renderPlaneYZ();

private:
    bool initialized;
    // Vertex Buffer Objects
    GLuint cubeVbo, sphereVbo, cylinderVbo, coneVbo, torusVbo, planeVbo;
    // Index Buffer Objects
    GLuint cubeIbo, sphereIbo, cylinderIbo, coneIbo, torusIbo, planeIbo;
    // Vertex Array Objects
    GLuint cubeVao, sphereVao, cylinderVao, coneVao, torusVao, planeVao;

    // Vertex Data
    static float  cubeVx[144], *sphereVx, *cylinderVx, *coneVx, *torusVx, planeVx[72];
    // Index Data
    static ushort cubeIx[36], *sphereIx, *cylinderIx, *coneIx, *torusIx, planeIx[18];

};
