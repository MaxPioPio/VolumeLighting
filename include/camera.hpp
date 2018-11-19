#pragma once

#include <QVector3d>
#include <QQuaternion>
#include <QMatrix4x4>

/**
 * A Camera is defined by a point of view,
 * a distance to this point of view (zoom) and an rotation
 * parameter describing the position on the sphere defined by
 * zoom and point of view.
 */
class Camera
{
public:
	Camera();
	~Camera();

    void translate(QVector3D v);
    void zoomDelta(float zoom);
    void rotate(QQuaternion q);

	void resizeFrame(int width, int height);
	void setProjection(bool perspective, float nearPlane, float farPlane, float fov);

    void resetPosition();

    QVector3D getPosition(); // = the pov
    QVector3D getEyePosition();
    float getZoom();
    QQuaternion getRotation();
    bool isPerspective();

	QMatrix4x4 *getViewMatrix();
	QMatrix4x4 *getProjectionMatrix();

protected:
    QVector3D pov;
	QQuaternion rotation;
    float zoom;

	float nearPlane, farPlane, fov, aspect;
	bool perspective;	// perspective or orthographic projection

private:
    bool viewMatDirty, projMatDirty;
	QMatrix4x4 *viewMat, *projMat;
};

