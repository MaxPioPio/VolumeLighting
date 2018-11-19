#pragma once

#include <QDockWidget>
#include <QCheckBox>
#include <QPushButton>

#include "transferfunction.hpp"
#include "transfunccanvas.hpp"
#include "volumedata.hpp"

class TransFuncEditor : public QDockWidget
{
    Q_OBJECT

public:
    TransFuncEditor(QWidget *parent, TransferFunction *tf, VolumeData *v);
    ~TransFuncEditor();


private:
    TransFuncCanvas *canvas;
    TransferFunction *transFunc;
    VolumeData *volume;

    QCheckBox *rBox, *gBox, *bBox, *aBox;
    QPushButton *smooth, *reset;

public slots:
    void updateCanvas();

private slots:
    void updateBoxes();
    void smoothTf();
    void defaultTf();

};