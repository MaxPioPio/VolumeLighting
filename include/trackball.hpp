#pragma once

#include <QtWidgets>

class Trackball
{
public:
	Trackball();
    Trackball(float radius);
	~Trackball();

	void reset();
    QVector3D getNormalFromTrackBall(float x, float y);
    QQuaternion move(QLineF delta);

    void setRadius(float radius);
    float getRadius();
	QQuaternion getQuaternion();
	
private:
	QQuaternion rotation;
    float radius;

};

