#pragma once

#include <QPointF>

#include "camera.hpp"
#include "primitives.hpp"
#include "scene.hpp"
#include "trackball.hpp"

class RenderWidget;
class ViewWidget;

class Controller : public QObject
{
    Q_OBJECT

public:

    static Controller* get();

    void setMV(Scene *scene, ViewWidget *view);

    void mousePress(RenderWidget* widget, QPointF screenPos, QPointF worldPos);
    void mouseRelease(RenderWidget* widget, QPointF screenPos, QPointF worldPos);
    void mouseMove(RenderWidget* widget, QLineF screenDelta, QLineF worldDelta, bool leftBtn, Qt::KeyboardModifiers mods);
    void mouseWheel(RenderWidget* widget, float delta, Qt::KeyboardModifiers mods);

private:
    Controller();

    Trackball trackball;
    bool simpleClick;
    QPointF startMousePosScreen;

    Scene *scene;
    ViewWidget *view;

    static Controller *controller;
};