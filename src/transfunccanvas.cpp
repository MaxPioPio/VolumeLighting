#include "transfunccanvas.hpp"

#include <QMouseEvent>
#include <QPainter>
#include <QDebug>

static const int WIDTH = 256, HEIGHT = 256;

TransFuncCanvas::TransFuncCanvas(QWidget *parent)
    : QWidget(parent)
{
    red = green = blue = alpha = false;
    this->setFixedSize(WIDTH, HEIGHT);
    tf = nullptr;
}

void TransFuncCanvas::setAffected(bool r, bool g, bool b, bool a) {
    red = r;
    green = g;
    blue = b;
    alpha = a;
}


void TransFuncCanvas::paintEvent(QPaintEvent* /*event*/) {

    if(!tf || !volume || !volume->isReady())
        return;

    QPainter paint(this);

    // Paint the background
    paint.fillRect(0, 0, WIDTH, HEIGHT, QBrush(QColor::fromHsv(0, 0, 220)));

    // Paint the Histogram
    paint.setPen(QColor::fromHsv(30, 40, 200));
    float *h = volume->createHistogram(WIDTH);
    for(int i=0; i<tf->getSize(); i++) {
        paint.drawLine(i, HEIGHT, i, HEIGHT - h[i] * HEIGHT);
    }

    // Draw the rgba functions
    QPointF rPrev(0, HEIGHT - tf->get(0).redF() * HEIGHT), rPos(0,0);
    QPointF gPrev(0, HEIGHT - tf->get(0).greenF() * HEIGHT), gPos(0,0);
    QPointF bPrev(0, HEIGHT - tf->get(0).blueF() * HEIGHT), bPos(0,0);
    QPointF aPrev(0, HEIGHT - tf->get(0).alphaF() * HEIGHT), aPos(0,0);
    QColor color;

    for(int i=0; i<tf->getSize(); i++) {
        color = tf->get(i);

        rPos.setX(i * WIDTH / tf->getSize());
        rPos.setY(HEIGHT - color.redF() * HEIGHT);
        paint.setPen(Qt::red);
        paint.drawLine(rPrev, rPos);

        gPos.setX(i * WIDTH / tf->getSize());
        gPos.setY(HEIGHT - color.greenF() * HEIGHT);
        paint.setPen(Qt::green);
        paint.drawLine(gPrev, gPos);

        bPos.setX(i * WIDTH / tf->getSize());
        bPos.setY(HEIGHT - color.blueF() * HEIGHT);
        paint.setPen(Qt::blue);
        paint.drawLine(bPrev, bPos);

        aPos.setX(i * WIDTH / tf->getSize());
        aPos.setY(HEIGHT - color.alphaF() * HEIGHT);
        paint.setPen(Qt::black);
        paint.drawLine(aPrev, aPos);

        rPrev = rPos;
        gPrev = gPos;
        bPrev = bPos;
        aPrev = aPos;
    }
}

void TransFuncCanvas::paintTf(TransferFunction *tf, VolumeData *volume) {
    this->tf = tf;
    this->volume = volume;
    update();
}

void TransFuncCanvas::changeTf(int x, int y) {
    int start = prevM.x() * tf->getSize() / WIDTH;
    int end = x * tf->getSize() / WIDTH;

    float vL, vR;
    if(end < start) {
        int t = end;
        end = start;
        start = t;

        vR = 1.f - prevM.y() / (float) HEIGHT;
        vL = 1.f - y / (float) HEIGHT;
    } else {
        vR = 1.f - y / (float) HEIGHT;
        vL = 1.f - prevM.y() / (float) HEIGHT;
    }

    float w = end - start + 1, v;
    for(int i = start; i <= end; i++) {
        if(i < 0 || i >= tf->getSize())
            continue;

        v = vL * (1.f - (i - start)/w) + vR * (i - start)/w;
        if(v < 0.f)
            v = 0.f;
        if(v > 1.f)
            v = 1.f;

        QColor c = tf->get(i);
        if(red)
            c.setRedF(v);
        if(green)
            c.setGreenF(v);
        if(blue)
            c.setBlueF(v);
        if(alpha)
            c.setAlphaF(v);

        tf->set(i, c);
    }
    prevM.setX(x);
    prevM.setY(y);
}

void TransFuncCanvas::mousePressEvent(QMouseEvent *event) {
    prevM = event->pos();
    changeTf(event->pos().x(), event->pos().y());
}

void TransFuncCanvas::mouseMoveEvent(QMouseEvent *event) {
    changeTf(event->pos().x(), event->pos().y());
}
