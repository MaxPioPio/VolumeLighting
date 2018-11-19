#include "trackball.hpp"

static const float MOVE_SNAP = 0.2f;

Trackball::Trackball()
{
	rotation = QQuaternion();
    radius = 1.f;
}

Trackball::Trackball(float radius)
{
    rotation = QQuaternion();
    this->radius = radius;
}


Trackball::~Trackball()
{
}

void Trackball::reset() {
	rotation = QQuaternion();
}

/**
 Calculates the normal on the trackball sheet for the position (x,y).
 The sheet is centered in the origin (0,0) and consits mainly of a sphere
 with radius radius in the middle. Points outside the sphere are mapped to 
 a surrounding hyperbolic sheet.
 See https://www.opengl.org/wiki/Object_Mouse_Trackball for more info.

 @return the normal on the trackball sheet on position x,y
*/
QVector3D Trackball::getNormalFromTrackBall(float x, float y) {
	const float radPow = radius*radius;
	float xPow = x*x, yPow = y*y;

	QVector3D ret;

    // points in the center are mapped to the sphere
	if (xPow + yPow < radPow / 2)
		ret = QVector3D(x, y, sqrt(radPow - (xPow + yPow)));
	// points outside the trackball sphere are mapped to a hyperbolic sheet
	else
		ret = QVector3D(x, y, radPow / 2 / sqrt(xPow + yPow));

	ret.normalize();
	return ret;
}

/**
 Calculates the rotation defined by the user input delta for this trackball
 and accumulates it to the stored rotation data.

 @param delta a line from the last to the current mouse position
*/
QQuaternion Trackball::move(QLineF delta) {
	// calculate the normal vectors for the last and current position on the trackball sheet
    QVector3D prevPos = getNormalFromTrackBall(delta.x1(), delta.y1());
	prevPos.normalize();
    QVector3D curPos = getNormalFromTrackBall(delta.x2(), delta.y2());
	curPos.normalize();

	// add the rotation difference between the normals to the rotation quaternion
	float angle = acos(QVector3D::dotProduct(prevPos, curPos));
    QQuaternion stepRotation = QQuaternion::fromAxisAndAngle(QVector3D::crossProduct(prevPos, curPos), qRadiansToDegrees(angle));
    rotation =  stepRotation * rotation;

    return stepRotation;
}

void Trackball::setRadius(float radius)
{
    this->radius = radius;
}

float Trackball::getRadius()
{
    return radius;
}

QQuaternion Trackball::getQuaternion()
{
	return rotation;
}
