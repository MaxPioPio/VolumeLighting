#include "viewwidget.hpp"


ViewWidget::ViewWidget()
{

    layout = new QVBoxLayout(this);

    // setup the render widgets
    perspectiveWidget = new RenderWidget(true);
    frontWidget = new RenderWidget(true);
    leftWidget = new RenderWidget(true);
    //leftWidget->getCamera()->rotate(QQuaternion::fromEulerAngles(90.f, 0.f, 0.f));
    topWidget = new RenderWidget(true);
    //topWidget->getCamera()->rotate(QQuaternion::fromEulerAngles(0.f, 90.f, 0.f));

    // connect the RenderWidget's clicked inside signals to let them request their selection
    connect(perspectiveWidget, SIGNAL(clickedInside()), this, SLOT(selectPerspective()));
    connect(frontWidget, SIGNAL(clickedInside()), this, SLOT(selectFront()));
    connect(leftWidget, SIGNAL(clickedInside()), this, SLOT(selectLeft()));
    connect(topWidget, SIGNAL(clickedInside()), this, SLOT(selectTop()));

    // setup the view layout
    quadViews = new QSplitter(Qt::Vertical, this);
    topViews = new QSplitter(Qt::Horizontal);
    topViews->addWidget(perspectiveWidget);
    topViews->addWidget(frontWidget);
    quadViews->addWidget(topViews);
    bottomViews = new QSplitter(Qt::Horizontal);
    bottomViews->addWidget(leftWidget);
    bottomViews->addWidget(topWidget);
    quadViews->addWidget(bottomViews);

    bottomViews->setChildrenCollapsible(false);
    topViews->setChildrenCollapsible(false);
    quadViews->setChildrenCollapsible(false);
    connect(bottomViews, SIGNAL(splitterMoved(int,int)), this, SLOT(syncTop()));
    connect(topViews, SIGNAL(splitterMoved(int,int)), this, SLOT(syncBottom()));

    layout->addWidget(quadViews);
    this->setLayout(layout);

    // start with the single view
    selectPerspective();
    quadViews->show();
    syncTop();
    currentView = QUAD_VIEW;
    switchToView(SINGLE_VIEW);
}


ViewWidget::~ViewWidget()
{
    delete perspectiveWidget;
    delete frontWidget;
    delete leftWidget;
    delete topWidget;
    delete topViews;
    delete bottomViews;
    delete quadViews;
    delete layout;
}

/**
 * @brief connects all four render widgets with the model
 * @param scene the data model
 */
void ViewWidget::setScene(Scene *scene) {
    perspectiveWidget->setScene(scene);
    frontWidget->setScene(scene);
    leftWidget->setScene(scene);
    topWidget->setScene(scene);
}

/**
 * @brief Switch to SINGLE-, DUAL- or QUAD_VIEW
 * @param view the new view mode
 */
void ViewWidget::switchToView(int view) {
    // hide all renderWidgets instead of
    // the perspective one
    switch(currentView) {
    case DUAL_VIEW:
        frontWidget->hide();
        break;
    case QUAD_VIEW:
        frontWidget->hide();
        bottomViews->hide();
    }

    // save the new view mode id
    currentView = view;

    // show the needed widgets for the new view mode
    if(view == DUAL_VIEW) {
        frontWidget->show();
    }
    else if(view == QUAD_VIEW) {
        frontWidget->show();
        bottomViews->show();
    }

    // if the active widget is now hidden
    // select the perspective widget (upper left)
    if(activeWidget->isHidden())
        selectPerspective();
}

/**
 * @brief updates all currently shown RenderWidgets
 */
void ViewWidget::updateActiveViews() {
    perspectiveWidget->update();
    if(currentView != SINGLE_VIEW)
        frontWidget->update();
    if(currentView == QUAD_VIEW) {
        topWidget->update();
        leftWidget->update();
    }
}

/**
 * @brief Sets the given widget as the new "ative" widget.
 * @param widget the widget to select
 */
void ViewWidget::select(RenderWidget *widget) {
    perspectiveWidget->deselect();
    frontWidget->deselect();
    topWidget->deselect();
    leftWidget->deselect();

    activeWidget = widget;
    widget->select();

    updateActiveViews();
}

RenderWidget* ViewWidget::getActiveWidget() {
    return activeWidget;
}


/* SLOTS ********************************************************* */

void ViewWidget::singleView() {
    switchToView(SINGLE_VIEW);
}
void ViewWidget::dualView() {
    switchToView(DUAL_VIEW);
}
void ViewWidget::quadView() {
    switchToView(QUAD_VIEW);
}

void ViewWidget::selectPerspective() {
    select(perspectiveWidget);
}
void ViewWidget::selectFront() {
    select(frontWidget);
}
void ViewWidget::selectLeft() {
    select(leftWidget);
}
void ViewWidget::selectTop() {
    select(topWidget);
}

void ViewWidget::homePosition()
{
    activeWidget->homePosition();
}

void ViewWidget::toggleCameraRotation() {
    perspectiveWidget->toggleCameraRotation();
    if(currentView != SINGLE_VIEW)
        frontWidget->toggleCameraRotation();
    if(currentView == QUAD_VIEW) {
        topWidget->toggleCameraRotation();
        leftWidget->toggleCameraRotation();
    }
}

// this Slots are used to sync the top and bottom
// positions of the horizontal sliders for the quad view
void ViewWidget::syncBottom() {
    bottomViews->setSizes(topViews->sizes());
}
void ViewWidget::syncTop() {
    topViews->setSizes(bottomViews->sizes());
}
