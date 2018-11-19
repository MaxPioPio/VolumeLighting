#pragma once

#include <QObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <QOpenGLFramebufferObject>
#include <QTimer>

#include "volumedata.hpp"
#include "volumerenderprops.hpp"
#include "camera.hpp"
#include "primitives.hpp"

// forward declaration
class ShadowRenderer;

class VolumeRenderer
        : public QObject
{
    Q_OBJECT

friend class ShadowRenderer;

public:
    VolumeRenderer(QOpenGLWidget *renderWiget, VolumeData *data, VolumeRenderProps *renderProps, int width, int height);
    ~VolumeRenderer();

    void resizeCanvas(int width, int height);

    void render(Camera *camera, PrimitiveUtils *primRenderer);

private:
    // the individual rendering steps
    void updateTransFuncFrom(TransferFunction *tf);
    void renderEntryExitPoints(Camera *camera, PrimitiveUtils *primRenderer);
    void renderVolume(Camera *camera, PrimitiveUtils *primRenderer);

    // the connected dataset
    VolumeData *dataset;
    // the connected volume render properties
    VolumeRenderProps *renderProps;
    // volume texture
    GLuint volumeTexture;

    // the connected transfer function
    TransferFunction *transFunc;
    GLuint transFuncTexture;
    bool tfTexDirty;

    // screen dimensions
    int width, height;

    // shader programs
    QOpenGLShaderProgram *entryExitShaderProg, *volumeShaderProg;

    // FrameBuffer w. 2 color attachements for entry exit points
    QOpenGLFramebufferObject *entryExitFBO;

    // the shadow renderer takes care of all render and OpenGL operations
    // for creating the shadow and opacity volumes
    ShadowRenderer *shadowRenderer;
    bool shadowVolumeReady;
    bool updateShadow;
    QTimer *timer;

    QOpenGLWidget *renderWidget;

    float scatteringTheta, scatteringPhi; // angles
    static const int SHADOW_UPDATE_DELAY = 400;

public slots:
    void datasetChanged();
    void transFuncChanged();
    void shadowPropsChanged();

private slots:
    void actualShadowUpdate();
};