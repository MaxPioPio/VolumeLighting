#pragma once

#include <QWidget>

#include "renderwidget.hpp"
#include "scene.hpp"

class ViewWidget :
    public QWidget
{
    Q_OBJECT

public:
    static const int SINGLE_VIEW = 0, DUAL_VIEW = 1, QUAD_VIEW = 2;

    ViewWidget();
    ~ViewWidget();

    void setScene(Scene *scene);

    RenderWidget* getActiveWidget();

private:
    void switchToView(int view);
    void select(RenderWidget *view);

    QVBoxLayout *layout;
    QSplitter *quadViews, *topViews, *bottomViews;

    RenderWidget *perspectiveWidget, *frontWidget, *leftWidget, *topWidget;
    int currentView;
    RenderWidget *activeWidget;

public slots:
    void updateActiveViews();
    void homePosition();
    void toggleCameraRotation();

    void singleView();
    void dualView();
    void quadView();

    void selectPerspective();
    void selectFront();
    void selectLeft();
    void selectTop();

private slots:
    void syncBottom();
    void syncTop();

};

