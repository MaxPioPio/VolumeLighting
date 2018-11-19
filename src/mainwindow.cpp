#include "mainwindow.hpp"

#define SLIDER_TICKS 1000

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    tfEditor = nullptr;

    setWindowTitle("Volume Visualization");
    resize(START_WIDTH, START_HEIGHT);

    // create the scene
    scene = new Scene();

    // create the view widget
    viewWidget = new ViewWidget();
    viewWidget->setScene(scene);
    // when volume render properties change, update the views
    connect(scene->getVolumeRenderProps(), SIGNAL(volumePropsChanged()), viewWidget, SLOT(updateActiveViews()));

    // obtain the controller
    Controller* controller = Controller::get();
    controller->setMV(scene, viewWidget);

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // Menu Bar
    menuBar = new QMenuBar();
    // Tool Bars
    mainToolBar = addToolBar("Tools");
    lightToolBar = new QToolBar("Lighting", this);
    lightToolBar->setAllowedAreas(Qt::LeftToolBarArea | Qt::RightToolBarArea);
    addToolBar(Qt::RightToolBarArea, lightToolBar);
    // Status Bar
    statusBar = new QStatusBar();

    // set up the file menu
    fileMenu = new QMenu("&File");
    menuBar->addMenu(fileMenu);

    // View Actions / General
    initGeneralGui();

    // Volume Rendering
    initVolumeRenderingGui();

    // add the exit action to the file menu
    exitAction = new QAction(QString("E&xit"), fileMenu);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));
    fileMenu->addAction(exitAction);

    // set up the about box
    aboutAction = new QAction(QString("&About"), nullptr);
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(showAboutBox()));
    menuBar->addAction(aboutAction);

    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // Add all elements to the window
    setMenuBar(menuBar);
    setStatusBar(statusBar);

    // add the central view widget
    setCentralWidget(viewWidget);

    // set all gui elements to the correct states
    updateGUIElems();
}

void MainWindow::closeEvent(QCloseEvent* /*event*/) {
    if(tfEditor)
        tfEditor->close();
    close();
}

MainWindow::~MainWindow()
{
}

void MainWindow::initGeneralGui() {
    // add the project loading button
    openProjAction = new QAction(QString("Open Project"), nullptr);
    openProjAction->setIcon(QIcon(":/resources/folder.png"));
    connect(openProjAction, SIGNAL(triggered()), this, SLOT(openProject()));
    mainToolBar->addAction(openProjAction);
    fileMenu->addAction(openProjAction);
    // add the project saving button
    saveProjAction = new QAction(QString("Save Project"), nullptr);
    saveProjAction->setIcon(QIcon(":/resources/save.png"));
    connect(saveProjAction, SIGNAL(triggered()), this, SLOT(saveProject()));
    mainToolBar->addAction(saveProjAction);
    fileMenu->addAction(saveProjAction);

    // add the home button
    homeAction = new QAction(QString("Camera Reset"), nullptr);
    homeAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_H));
    homeAction->setIcon(QIcon(":/resources/cam_home.png"));
    connect(homeAction, SIGNAL(triggered()), viewWidget, SLOT(homePosition()));
    mainToolBar->addAction(homeAction);

    // add the camera rotation button
    cameraRotationAction = new QAction(QString("Camera Rotation"), nullptr);
    cameraRotationAction->setIcon(QIcon(":/resources/rotation.png"));
    connect(cameraRotationAction, SIGNAL(triggered()), viewWidget, SLOT(toggleCameraRotation()));
    mainToolBar->addAction(cameraRotationAction);

    mainToolBar->addSeparator();

    // add the view layout buttons
    viewMenu = new QMenu();
    QActionGroup *viewGroup = new QActionGroup(this);
    singleViewAction = new QAction(QString("Single View"), nullptr);
        singleViewAction->setShortcut(QKeySequence(Qt::Key_1));
        singleViewAction->setIcon(QIcon(":/resources/view-single.png"));
        singleViewAction->setCheckable(true);
        singleViewAction->setChecked(true);
        viewGroup->addAction((singleViewAction));
        connect(singleViewAction, SIGNAL(triggered()), viewWidget, SLOT(singleView()));
    dualViewAction = new QAction(QString("Dual View"), nullptr);
        dualViewAction->setShortcut(QKeySequence(Qt::Key_2));
        dualViewAction->setIcon(QIcon(":/resources/view-dual.png"));
        dualViewAction->setCheckable(true);
        viewGroup->addAction((dualViewAction));
        connect(dualViewAction, SIGNAL(triggered()), viewWidget, SLOT(dualView()));
    quadViewAction = new QAction(QString("Quad View"), nullptr);
        quadViewAction->setShortcut(QKeySequence(Qt::Key_4));
        quadViewAction->setIcon(QIcon(":/resources/view-quad.png"));
        quadViewAction->setCheckable(true);
        viewGroup->addAction((quadViewAction));
        connect(quadViewAction, SIGNAL(triggered()), viewWidget, SLOT(quadView()));
    viewMenu->addActions(viewGroup->actions());
    QToolButton *btn = new QToolButton();
    btn->setToolTip("View Layouts");
    btn->setIcon(QIcon(":/resources/view-quad.png"));
    btn->setMenu(viewMenu);
    btn->setPopupMode(QToolButton::InstantPopup);
    mainToolBar->addWidget(btn);
}

void MainWindow::initVolumeRenderingGui() {
   mainToolBar->addSeparator();

   // add the action for loading a data set
   openVolumeAction = new QAction(QString("Open Volume"), nullptr);
   openVolumeAction->setIcon(QIcon(":/resources/volLoad.png"));
   connect(openVolumeAction, SIGNAL(triggered()), this, SLOT(openVolumeData()));
   mainToolBar->addAction(openVolumeAction);

   mainToolBar->addSeparator();

   //add the mode selector
   modeCombo = new QComboBox();
   modeCombo->insertItem(0, QString("DVR"));
   modeCombo->insertItem(1, QString("MIP"));
   modeCombo->insertItem(2, QString("Entry Points"));
   modeCombo->insertItem(3, QString("Exit Points"));
   modeCombo->insertItem(4, QString("Debug Box"));
   connect(modeCombo, SIGNAL(activated(int)), scene->getVolumeRenderProps(), SLOT(setMode(int)));
   mainToolBar->addWidget(modeCombo);

   // add the step size slider
   mainToolBar->addSeparator();
   stepSlider = new QSlider(Qt::Horizontal);
   stepSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
   stepSlider->setRange(1, SLIDER_TICKS);
   stepSlider->setValue(200);
   connect(stepSlider, SIGNAL(valueChanged(int)), this, SLOT(stepSizeMoved(int)));
   mainToolBar->addWidget(new QLabel(QString(" Step Size  ")));
   mainToolBar->addWidget(stepSlider);

   // add the actions for the transfer functions
   QMenu *tfMenu = new QMenu(QString("Transfer Function"));
   tfEditorAction = new QAction(QString("TF Editor"), nullptr);
       tfEditorAction->setIcon(QIcon(":/resources/tf.png"));
       connect(tfEditorAction, SIGNAL(triggered()), this, SLOT(showTfEditor()));
       tfMenu->addAction(tfEditorAction);
   loadTfAction = new QAction(QString("Load TF"), nullptr);
       loadTfAction->setIcon(QIcon(":/resources/tfLoad.png"));
       connect(loadTfAction, SIGNAL(triggered()), this, SLOT(loadTf()));
       tfMenu->addAction(loadTfAction);
   saveTfAction = new QAction(QString("Save TF"), nullptr);
       saveTfAction->setIcon(QIcon(":/resources/tfSave.png"));
       connect(saveTfAction, SIGNAL(triggered()), this, SLOT(saveTf()));
       tfMenu->addAction(saveTfAction);  

   menuBar->addMenu(tfMenu);
   mainToolBar->addSeparator();
   mainToolBar->addActions(tfMenu->actions());

   // LIGHTING ---------------------------------------------------------------------
   //add the lighting mode selector
   lightCombo = new QComboBox();
   lightCombo->insertItem(0, QString("No Lighting"));
   lightCombo->insertItem(1, QString("Phong"));
   lightCombo->insertItem(2, QString("Global"));
   lightCombo->insertItem(3, QString("Global + Phong"));
   connect(lightCombo, SIGNAL(activated(int)), scene->getVolumeRenderProps(), SLOT(setLightingMode(int)));
   lightToolBar->addWidget(lightCombo);

   // Light Source Position
   lightToolBar->addWidget(new QLabel("Light Position"));
   lightPosX = new QSlider(Qt::Horizontal);
   lightPosX->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
   lightPosX->setRange(0, SLIDER_TICKS);
   connect(lightPosX, SIGNAL(valueChanged(int)), this, SLOT(lightPosXMoved(int)));
   lightToolBar->addWidget(lightPosX);

   lightPosY = new QSlider(Qt::Horizontal);
   lightPosY->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
   lightPosY->setRange(0, SLIDER_TICKS);
   connect(lightPosY, SIGNAL(valueChanged(int)), this, SLOT(lightPosYMoved(int)));
   lightToolBar->addWidget(lightPosY);


   lightPosZ = new QSlider(Qt::Horizontal);
   lightPosZ->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
   lightPosZ->setRange(0, SLIDER_TICKS);
   connect(lightPosZ, SIGNAL(valueChanged(int)), this, SLOT(lightPosZMoved(int)));
   lightToolBar->addWidget(lightPosZ);

   // Light Source type
   lightDirBox = new QCheckBox("Directional");
   connect(lightDirBox, SIGNAL(clicked(bool)), scene->getVolumeRenderProps(), SLOT(setLightDirectional(bool)));
   lightToolBar->addWidget(lightDirBox);

   // Light Segment Length
   lightToolBar->addWidget(new QLabel("Opacitiy Approx."));
   lightSegmentSlider = new QSlider(Qt::Horizontal);
   lightSegmentSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
   lightSegmentSlider->setRange(1, SLIDER_TICKS);
   connect(lightSegmentSlider, SIGNAL(valueChanged(int)), this, SLOT(lightSegmentLengthMoved(int)));
   lightToolBar->addWidget(lightSegmentSlider);

   // Light Intensity
   lightToolBar->addWidget(new QLabel("Light Intensity"));
   lightIntensitySlider = new QSlider(Qt::Horizontal);
   lightIntensitySlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
   lightIntensitySlider->setRange(1, SLIDER_TICKS);
   connect(lightIntensitySlider, SIGNAL(valueChanged(int)), this, SLOT(lightIntensityMoved(int)));
   lightToolBar->addWidget(lightIntensitySlider);

   // Minimum Light Intensity
   lightToolBar->addWidget(new QLabel("Minimum Intensity"));
   lightBaseIntensitySlider = new QSlider(Qt::Horizontal);
   lightBaseIntensitySlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
   lightBaseIntensitySlider->setRange(0, SLIDER_TICKS);
   connect(lightBaseIntensitySlider, SIGNAL(valueChanged(int)), this, SLOT(lightBaseIntensityMoved(int)));
   lightToolBar->addWidget(lightBaseIntensitySlider);

   // Light Opacity Fall Off
   lightToolBar->addWidget(new QLabel("Optical Thickness"));
   lightFallOffSlider = new QSlider(Qt::Horizontal);
   lightFallOffSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
   lightFallOffSlider->setRange(0, SLIDER_TICKS);
   connect(lightFallOffSlider, SIGNAL(valueChanged(int)), this, SLOT(lightFallOffMoved(int)));
   lightToolBar->addWidget(lightFallOffSlider);

   // Light Scattering Radius
   lightToolBar->addWidget(new QLabel("Scattering Radius"));
   lightRadiusSlider = new QSlider(Qt::Horizontal);
   lightRadiusSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
   lightRadiusSlider->setRange(0, SLIDER_TICKS);
   connect(lightRadiusSlider, SIGNAL(valueChanged(int)), this, SLOT(lightScatteringRadiusMoved(int)));
   lightToolBar->addWidget(lightRadiusSlider);
}

void MainWindow::updateGUIElems() {
    VolumeRenderProps *props = scene->getVolumeRenderProps();
    stepSlider->setValue(props->getStepSizeN() * SLIDER_TICKS);
    modeCombo->setCurrentIndex(props->getMode());
    lightCombo->setCurrentIndex(props->getLightingMode());
    lightPosX->setValue(props->getLightPosXN() * SLIDER_TICKS);
    lightPosY->setValue(props->getLightPosYN() * SLIDER_TICKS);
    lightPosZ->setValue(props->getLightPosZN() * SLIDER_TICKS);
    lightDirBox->setChecked(props->getLightDirectional());
    lightIntensitySlider->setValue(props->getLightIntensityN() * SLIDER_TICKS);
    lightBaseIntensitySlider->setValue(props->getLightBaseIntensityN() * SLIDER_TICKS);
    lightSegmentSlider->setValue(props->getLightSegmentLengthN() * SLIDER_TICKS);
    lightFallOffSlider->setValue(props->getLightOpacityBaseStepN() * SLIDER_TICKS);
    lightRadiusSlider->setValue(props->getScatteringRadiusN() * SLIDER_TICKS);
}


// ####################### SLOTS ############################################

void MainWindow::showAboutBox()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("About Volume Visualization");
    msgBox.setText("This program shows an implementation of the global lighting algorithm propagated in 'Interactive Global Light Propagation in Direct Volume Rendering using Local Piecewise Integration', Hernell et. al. 2008."
                   "\n\nThe upper GUI offers functions to load and save whole projects, reset the camera, enable a continuous camera rotation and to select different view layouts."
                   "\nVolume datasets in a simple raw format can be loaded and displayed using default direct volume rendering or a maximum intensity projection. The sampling step size for the rendering operations is uniform but variable."
                   "\nOnedimensional transfer functions can be edited with a simple GUI and saved and loaded independently from a project file."
                   "\n\nThe GUI on the right side offers control over the lighting parameters. Local lighting is realized with a simple Phong model where gradients are approximated over forward differences."
                   "\nLight sources can be directional or point lights. The global lighting method propagated by Hernell et. al. can be seen in action when 'Global' lighting is selected."
                   "\nIn a first step the method approximates the light reaching every voxel by evaluation short opacity rays. Their length can be controlled with the 'Opacity Approx.' slider where the highest level evalutes the whole volume length with each shadow ray and offers the highest precision while being computationally intensive."
                   "\nThe intensity of the global light source is splitted in a normal intensity value and a 'Minimum Intensity' that influences every voxel regardless of its global opacity. The optical thickness of the material can be changed to obtain more meaningful shadows. Additionally to the shadow computation an approximization of scattering is implemented. It assumes that every voxel scatters a small amount of its incoming light equally in all directions to it's neighbouring voxels. The scattering amount can be controlled by defining the radius in which voxel contribute to each others light level."
                   "\n\n Max Piochowiak.");
    msgBox.exec();
}


void MainWindow::updateStatusBar(QString text) {
    statusBar->showMessage(text);
    viewWidget->updateActiveViews();
}

void MainWindow::openProject() {
    QString file = QFileDialog::getOpenFileName(this, QString("Open Project"), QString("../Projects"), QString("Project (*.prj)"));
    scene->openProject(file);
    updateGUIElems();
}

void MainWindow::saveProject() {
    QString file = QFileDialog::getSaveFileName(this, QString("Save Project"), QString("../Projects"), QString("Project (*.prj)"));
    scene->saveProject(file);
}

// Volume Rendering

void MainWindow::openVolumeData() {
    QString file = QFileDialog::getOpenFileName(this, QString("Open Volume Data Set"), QString("../VolumeData"), QString("Volume Data (*.raw)"));
    scene->loadVolume(file);
}

void MainWindow::showTfEditor() {
    if(!tfEditor) {
        tfEditor = new TransFuncEditor(nullptr, scene->getVolumeRenderProps()->getTransFunc(), scene->getVolume());
        connect(scene->getVolumeRenderProps()->getTransFunc(), SIGNAL(transFuncChanged()), tfEditor, SLOT(updateCanvas()));
    }

    tfEditor->show();
    tfEditor->raise();
}

void MainWindow::saveTf() {
    QString path = QFileDialog::getSaveFileName(this, QString("Save Transfer Function"), QString("../VolumeVisualization/TransFuncs"), QString("Transfer Function (*.tf)"));

    // open the file
    QFile file(path);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "Could not create file '" << path << "'' !";
        return;
    }

    QDataStream out(&file);

    scene->getVolumeRenderProps()->getTransFunc()->saveTo(out);

    file.close();
}

void MainWindow::loadTf() {
    QString path = QFileDialog::getOpenFileName(this, QString("Load Transfer Function"), QString("../VolumeVisualization/TransFuncs"), QString("Transfer Function (*.tf)"));
    // check if the file exists
    QFile file(path);
    if(!file.exists()) {
        qWarning() << "File '" << path << "'' does not exist!";
        return;
    }

    // open the file
    if(!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open file '" << path << "'' !";
        return;
    }

    QDataStream in(&file);
    scene->getVolumeRenderProps()->getTransFunc()->loadFrom(in);
    file.close();
}


void MainWindow::stepSizeMoved(int v) {
   scene->getVolumeRenderProps()->setStepSize(static_cast<float>(v)/SLIDER_TICKS);
}

// Light -------------------------------------

void MainWindow::lightPosXMoved(int v) {
    scene->getVolumeRenderProps()->setLightPosX(static_cast<float>(v)/SLIDER_TICKS);
}

void MainWindow::lightPosYMoved(int v) {
    scene->getVolumeRenderProps()->setLightPosY(static_cast<float>(v)/SLIDER_TICKS);
}

void MainWindow::lightPosZMoved(int v) {
    scene->getVolumeRenderProps()->setLightPosZ(static_cast<float>(v)/SLIDER_TICKS);
}

void MainWindow::lightFallOffMoved(int v) {
    scene->getVolumeRenderProps()->setLightOpacityBaseStep(static_cast<float>(v)/SLIDER_TICKS);
}

void MainWindow::lightIntensityMoved(int v) {
    scene->getVolumeRenderProps()->setLightIntensity(static_cast<float>(v)/SLIDER_TICKS);
}

void MainWindow::lightBaseIntensityMoved(int v) {
    scene->getVolumeRenderProps()->setLightBaseIntensity(static_cast<float>(v)/SLIDER_TICKS);
}

void MainWindow::lightSegmentLengthMoved(int v) {
    scene->getVolumeRenderProps()->setLightSegmentLength(static_cast<float>(v)/SLIDER_TICKS);
}

void MainWindow::lightScatteringRadiusMoved(int v) {
    scene->getVolumeRenderProps()->setScatteringRadius(static_cast<float>(v)/SLIDER_TICKS);
}


// Keyboard --------------------

//void MainWindow::keyPressEvent(QKeyEvent *event) {
//    QMainWindow::keyPressEvent(event);
//    Controller::get()->keyPress(event->key());
//}

//void MainWindow::keyReleaseEvent(QKeyEvent *event) {
//    QMainWindow::keyReleaseEvent(event);
//    Controller::get()->keyRelease(event->key());
//}
