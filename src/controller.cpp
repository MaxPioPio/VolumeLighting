#include "controller.hpp"

#include "renderwidget.hpp"
#include "trackball.hpp"
#include "viewwidget.hpp"


static const float WHEEL_FACTOR = -0.0002f;
static const float MOVE_SPEED = 2.f;

Controller* Controller::controller;

Controller::Controller() {
    trackball = Trackball(1.f);
    simpleClick = false;
    startMousePosScreen = QPointF();
}

/**
 * @brief returns the controller instance of this singleton
 * @return the Controller instance
 */
Controller* Controller::get() {
    if(!controller)
        controller = new Controller();
    return controller;
}

/**
 * @brief To tell the controller which model and view to use.
 * @param scene the model/scene class
 * @param view the view class
 */
void Controller::setMV(Scene *scene, ViewWidget *view) {
    this->scene = scene;
    this->view = view;
}

void Controller::mousePress(RenderWidget*, QPointF screenPos, QPointF) {
    simpleClick = true;
    startMousePosScreen = screenPos;
}

void Controller::mouseRelease(RenderWidget* /*widget*/, QPointF /*screenPos*/, QPointF /*worldPos*/) {
    if(simpleClick) {
        simpleClick = false;

        // place actions for a single mouseclick here
    }
}

float minElem(QVector3D v) {
    if (abs(v.x()) > abs(v.y())) {
        if(abs(v.x()) > abs(v.z()))
            return v.x();
        else
            return v.z();
    } else {
        if(abs(v.y()) > abs(v.z()))
            return v.y();
        else
            return v.z();
    }
}

/**
 * @brief Moving the cursor can rotate or translate the active camera.
 */
void Controller::mouseMove(RenderWidget *widget, QLineF screenDelta, QLineF, bool leftBtn, Qt::KeyboardModifiers /*mods*/) {

    simpleClick = false;
//    bool ctrl = mods & Qt::CTRL;
//    bool shift = mods & Qt::ShiftModifier;

    Camera *camera = widget->getCamera();
    // rotate the camera with the left button
    if (leftBtn && camera->isPerspective()) {
        trackball.reset();
        QQuaternion q = trackball.move(QLineF(screenDelta.p1() - startMousePosScreen, screenDelta.p2() - startMousePosScreen));
        camera->rotate(-q);
    }
    // translate the camera with the right mouse button
    else if(!leftBtn) {
        QVector3D deltaVec(screenDelta.dx(), screenDelta.dy(), 0.f);
        camera->translate(deltaVec);
    }

    widget->update();
}

/**
 * @brief the mouse wheel changes the camera zoom
 */
void Controller::mouseWheel(RenderWidget *widget, float delta, Qt::KeyboardModifiers /*mods*/) {
//    bool ctrl = mods & Qt::CTRL;
//    bool shift = mods & Qt::ShiftModifier;

    widget->getCamera()->zoomDelta(delta * WHEEL_FACTOR);
    widget->update();
}
