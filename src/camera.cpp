#include "camera.hpp"

static const float MIN_ZOOM = 0.8f, MAX_ZOOM = 20.f;

Camera::Camera() {
	viewMat = new QMatrix4x4();
	projMat = new QMatrix4x4();
    resetPosition();

	aspect = 1.f;
    setProjection(false, 0.01f, 10.f, 45.f);
}

Camera::~Camera() {
	delete viewMat;
	delete projMat;
}

/**
 * @brief Moves the camera in xy-direction based on its current orientation
 * @param v the delta translation vector
 */
void Camera::translate(QVector3D v)
{
    // we can only move "left/right"
    v.setZ(0);

    // if we're far away from the pov,
    // move further
    if(zoom > 1.f)
        v *= zoom/2.f;

    pov -= rotation * v;
    viewMatDirty = true;
}

/**
 * @brief zooms the camera by zoom units
 */
void Camera::zoomDelta(float zoom) {
    this->zoom += zoom;

    // respect the given constraints
    if(this->zoom < MIN_ZOOM)
        this->zoom = MIN_ZOOM;
    else if(this->zoom > MAX_ZOOM)
        this->zoom = MAX_ZOOM;

    // different zoom values need different orthographic (!) projections
    if(!perspective)
        projMatDirty = true;

    viewMatDirty = true;
}

/**
 * @param q the rotation that will be applied to this camera
 */
void Camera::rotate(QQuaternion q) {
    rotation = QQuaternion(q.scalar(), rotation * q.vector()) * rotation;
	viewMatDirty = true;
}

/**
 * @brief Updates the cameras projectionmatrix and aspect to the new frame
 */
void Camera::resizeFrame(int width, int height) {
	aspect = ((float)width) / height;
    projMatDirty = true;
}

/**
 * @brief sets the projection matrix of this camera. Only relevant in perspective mode.
 */
void Camera::setProjection(bool perspective, float nearPlane, float farPlane, float fov = -1) {
	this->perspective = perspective;
	this->nearPlane = nearPlane;
	this->farPlane = farPlane;
	if(fov > 0)
		this->fov = fov;

    projMatDirty = true;
}

void Camera::resetPosition() {
    zoom = 2.f;
    pov = QVector3D();
    rotation = QQuaternion();
    viewMatDirty = projMatDirty = true;
}

QVector3D Camera::getPosition() {
    return pov;
}

QVector3D Camera::getEyePosition() {
    return rotation * QVector3D(0, 0, zoom) + pov;
}

float Camera::getZoom() {
    return zoom;
}

QQuaternion Camera::getRotation() {
    return rotation;
}

bool Camera::isPerspective() {
    return perspective;
}

QMatrix4x4 * Camera::getViewMatrix() {
	if (viewMatDirty) {
		viewMat->setToIdentity();
        viewMat->lookAt(getEyePosition(), pov, rotation * QVector3D(0, 1, 0));
		
		viewMatDirty = false;
	}

	return viewMat;
}

QMatrix4x4 * Camera::getProjectionMatrix() {
    if(projMatDirty) {

        projMat->setToIdentity();
        // perspective
        if(perspective) {
            projMat->perspective(fov, aspect, nearPlane, farPlane);
        }
        // orthographic
        else {
            float w = 1.f*zoom;
            if(aspect < 1.f)
                projMat->ortho(-w, w, -w/aspect, w/aspect, nearPlane/w, farPlane*w);
            else
                projMat->ortho(-w*aspect, w*aspect, -w, w, nearPlane/w, farPlane*w);
        }

        projMatDirty = false;
    }
	return projMat;
}
