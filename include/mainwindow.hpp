#pragma once

#include <QMainWindow>
#include <QItemSelectionModel>

#include "viewwidget.hpp"
#include "transfunceditor.hpp"
#include "renderwidget.hpp"
#include "controller.hpp"


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    static const int START_WIDTH = 640, START_HEIGHT = 480;

    explicit MainWindow(QWidget *parent = 0);
    void updateGUIElems();
    ~MainWindow();

private:
    // Scene
    Scene *scene;
    // View
    ViewWidget *viewWidget;

    QMenuBar *menuBar;
    QToolBar *mainToolBar, *lightToolBar;
    QMenu *fileMenu;
    QStatusBar *statusBar;

    // Menu bar Actions
    QAction *aboutAction, *exitAction;
    QAction *openProjAction, *saveProjAction;

    // View Actions
    QMenu *viewMenu;
    QAction *singleViewAction, *dualViewAction, *quadViewAction;
    QAction *homeAction, *cameraRotationAction;

    // Volume Data Actions
    QAction *openVolumeAction;
    // Sampling Step Slider
    QSlider *stepSlider;
    // Render Mode Selection
    QComboBox *modeCombo;
    // Transfer Function Actions
    QAction *tfEditorAction, *saveTfAction, *loadTfAction;

    // Lighting Mode Selection
    QComboBox *lightCombo;
    // Light Source Position
    QSlider *lightPosX, *lightPosY, *lightPosZ;
    // Light Source type
    QCheckBox *lightDirBox;
    // Light Intensity
    QSlider *lightIntensitySlider;
    QSlider *lightBaseIntensitySlider;
    // Light Segment Length
    QSlider *lightSegmentSlider;
    // Light Opacity Fall Off
    QSlider *lightFallOffSlider;
    // Light Scattering Radius
    QSlider *lightRadiusSlider;

    // Transfer Function Editor
    TransFuncEditor *tfEditor;

    // View Actions / General
    void initGeneralGui();
    //Volume Rendering
    void initVolumeRenderingGui();

protected:
     void closeEvent(QCloseEvent *event);

public slots:
    // general
    void showAboutBox();
    void updateStatusBar(QString text);
    void openProject();
    void saveProject();
    // volume rendering
    void openVolumeData();
    void showTfEditor();
    void saveTf();
    void loadTf();
    void stepSizeMoved(int v);
    // lighting
    void lightPosXMoved(int v);
    void lightPosYMoved(int v);
    void lightPosZMoved(int v);
    void lightFallOffMoved(int v);
    void lightIntensityMoved(int v);
    void lightBaseIntensityMoved(int v);
    void lightSegmentLengthMoved(int v);
    void lightScatteringRadiusMoved(int v);
};