#include "transferfunction.hpp"

#include <QDebug>
#include <QFile>
#include <QDataStream>

#include "glutils.hpp"

TransferFunction::TransferFunction()
{
    size = 256;
    colors = new QColor[size];
    defaultTf(0);
}

TransferFunction::TransferFunction(int size) {
    this->size = size;
    colors = new QColor[size];
    defaultTf(0);
}

TransferFunction::~TransferFunction() {
    delete[] colors;
}

float* TransferFunction::toData() {
    float *data = new float[size * 4];
    for(int i=0; i<size*4; i+=4) {
        data[i]  = colors[i/4].redF();
        data[i+1]  = colors[i/4].greenF();
        data[i+2]  = colors[i/4].blueF();
        data[i+3]  = colors[i/4].alphaF();
    }
    return data;
}


QColor* TransferFunction::getAll() {
    return colors;
}

QColor TransferFunction::get(float intensity) {
    if(intensity < 0 || intensity > 1.f)  {
        qWarning() << "Transfer function color id > size!";
        return QColor();
    }

    return colors[(int) (intensity * (size-1))];
}

QColor TransferFunction::get(int id) {
    if(id < 0 || id >= size)  {
        qWarning() << "Transfer function color id > size!";
        return QColor();
    }

    return colors[id];
}

void TransferFunction::set(float intensity, QColor color) {
    if(intensity < 0 || intensity > 1.f)  {
        qWarning() << "Transfer function intensity outside borders!";
        return;
    }

    int id = (int) (intensity * (size-1));
    set(id, color);
}

void TransferFunction::set(int id, QColor color) {
    if(id < 0 || id >= size)  {
        qWarning() << "Transfer function color id > size!";
        return;
    }

    bool alphaChanged = colors[id].alpha() != color.alpha();
    colors[id] = color;

    emit transFuncChanged();
    if(alphaChanged)
        emit transFuncChangedAlpha();
}

void TransferFunction::defaultTf(int type, bool r, bool g, bool b, bool a) {

    QColor cNew;

    for(int i=0; i<size; i++) {
        switch(type) {
        case 1:
            cNew = QColor::fromHsv(0, 0, i*255/size, 20);
            break;
        case 2:
            cNew = QColor::fromHsv(i*255/size, 255, 255, i*255/size);
            break;
        default:
            cNew = QColor::fromHsv(0, 0, i*255/size, i*255/size);
            break;
        }

        if(r)
            colors[i].setRed(cNew.red());
        if(g)
            colors[i].setGreen(cNew.green());
        if(b)
            colors[i].setBlue(cNew.blue());
        if(a)
            colors[i].setAlpha(cNew.alpha());

    }

    emit transFuncChanged();
    if(a)
        emit transFuncChangedAlpha();
}

/**
 * @brief smoothes this transfer function with a global gaussian filter
 */
void TransferFunction::smooth(bool cr, bool cg, bool cb, bool ca) {

    float r,g,b,a;
    QColor *cNew =  new QColor[size];

    // pre-compute 2*sigma^2
    float sigma = sqrt(size)/3.f;
    sigma = 2*sigma*sigma;

    // if we keep track of the sum of the weights (= wSum)
    // we don't have to use the normalizing factor
    // 1/(2*pi*sigma^2) in every iteration
    float w, wSum;

    for(int i = 0; i < size; i++) {

        r = g = b = a = 0;
        wSum = 0;

        // weigh all tf values by their distance to the
        // currently affected value
        for(int j = 0; j < size; j++) {
            w = exp(-(i-j)*(i-j)/sigma);
            r += colors[j].redF() * w;
            g += colors[j].greenF() * w;
            b += colors[j].blueF() * w;
            a += colors[j].alphaF() * w;

            wSum += w;
        }

        // normalize the new color by wSum
        cNew[i] = colors[i];
        if(cr)
            cNew[i].setRedF(r/wSum);
        if(cg)
            cNew[i].setGreenF(g/wSum);
        if(cb)
            cNew[i].setBlueF(b/wSum);
        if(ca)
            cNew[i].setAlphaF(a/wSum);
    }

    delete[] colors;
    colors = cNew;

    emit transFuncChanged();
    if(ca)
        emit transFuncChangedAlpha();
}


int TransferFunction::getSize() {
    return size;
}

void TransferFunction::saveTo(QDataStream &out) {
    // write size and color data to file
    out << size;
    for(int i=0; i<size; i++)
        out << colors[i];
}

void TransferFunction::loadFrom(QDataStream &in) {
    // Read size and color data from file
    delete[] colors;

    in >> size;
    colors = new QColor[size];
    for(int i=0; i<size; i++)
        in >> colors[i];

    emit transFuncChangedAlpha();
    emit transFuncChanged();
}
