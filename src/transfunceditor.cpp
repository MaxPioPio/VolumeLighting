#include "transfunceditor.hpp"

#include <QGridLayout>
#include <QLabel>

TransFuncEditor::TransFuncEditor(QWidget *parent, TransferFunction *tf, VolumeData *v)
    : QDockWidget(parent) {

    QGridLayout *layout = new QGridLayout();

    // add the canvas
    canvas = new TransFuncCanvas(this);
    layout->addWidget(canvas, 0, 0, 1, 4);
    canvas->show();
    canvas->paintTf(tf, v);

    // add the check boxes
    rBox = new QCheckBox("Red", this);
    connect(rBox, SIGNAL(clicked(bool)), this, SLOT(updateBoxes()));
    layout->addWidget(rBox, 1, 0);
    gBox = new QCheckBox("Green", this);
    connect(gBox, SIGNAL(clicked(bool)), this, SLOT(updateBoxes()));
    layout->addWidget(gBox, 1, 1);
    bBox = new QCheckBox("Blue", this);
    connect(bBox, SIGNAL(clicked(bool)), this, SLOT(updateBoxes()));
    layout->addWidget(bBox, 1, 2);
    aBox = new QCheckBox("Alpha", this);
    connect(aBox, SIGNAL(clicked(bool)), this, SLOT(updateBoxes()));
    layout->addWidget(aBox, 1, 3);

    // add smooth button
    smooth = new QPushButton(QString("Smooth"), this);
    connect(smooth, SIGNAL(clicked(bool)), this, SLOT(smoothTf()));
    layout->addWidget(smooth, 2, 0, 1, 2);

    // add defualt tf button
    reset = new QPushButton(QString("Reset"), this);
    connect(reset, SIGNAL(clicked(bool)), this, SLOT(defaultTf()));
    layout->addWidget(reset, 2, 2, 1, 2);

    QWidget *widget = new QWidget(this);
    widget->setLayout(layout);
    this->setWidget(widget);
    this->transFunc = tf;
    this->volume = v;
}

TransFuncEditor::~TransFuncEditor() {
    delete canvas;
}

void TransFuncEditor::updateCanvas() {
    canvas->paintTf(transFunc, volume);
}

void TransFuncEditor::updateBoxes() {
    canvas->setAffected(rBox->isChecked(), gBox->isChecked(), bBox->isChecked(), aBox->isChecked());
}

void TransFuncEditor::smoothTf() {
    transFunc->smooth(rBox->isChecked(), gBox->isChecked(), bBox->isChecked(), aBox->isChecked());
}

void TransFuncEditor::defaultTf() {
    transFunc->defaultTf(0, rBox->isChecked(), gBox->isChecked(), bBox->isChecked(), aBox->isChecked());
}
