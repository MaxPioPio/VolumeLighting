#pragma once

#include <QtWidgets>
#include <QOpenGLFunctions_4_0_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLFramebufferObject>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <QTimer>

#include "camera.hpp"
#include "controller.hpp"
#include "primitives.hpp"
#include "scene.hpp"
#include "trackball.hpp"
#include "volumerenderer.hpp"


class RenderWidget :
    public QOpenGLWidget
{
	Q_OBJECT

public:    

    RenderWidget(bool perspective = false);
	~RenderWidget();

    void setCamera(Camera *camera);
    Camera* getCamera();
    void setScene(Scene *scene);

    void select();
    void deselect();

protected:

    // overloaded OpenGL
	void initializeGL();
	void resizeGL(int width, int height);
	void paintGL();

    // mouse input
	QPointF screenToWorld(QPointF p);
	void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent *event);
	
    QPointF touchPos, touchPrev, touchPosS, touchPrevS;

    Camera *camera;
    Scene *scene;
    bool selected;

    PrimitiveUtils *primRenderer;
    VolumeRenderer *volumeRenderer;

    QTimer *rotationTimer;

public slots:
	void homePosition();
    void toggleCameraRotation();

private slots:
    void rotationTimerEnd();

signals:
    void clickedInside();

};

