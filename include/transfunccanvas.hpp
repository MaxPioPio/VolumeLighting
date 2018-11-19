#pragma once

#include <QWidget>

#include "transferfunction.hpp"
#include "volumedata.hpp"

class TransFuncCanvas : public QWidget
{

public:
    TransFuncCanvas(QWidget *parent);
    void paintTf(TransferFunction *tf, VolumeData* volume);

    void setAffected(bool r, bool g, bool b, bool a);

protected:
    void paintEvent(QPaintEvent *event);
    void changeTf(int x, int y);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    QPoint prevM;
    TransferFunction *tf;
    VolumeData *volume;
    bool red, green, blue, alpha;
};