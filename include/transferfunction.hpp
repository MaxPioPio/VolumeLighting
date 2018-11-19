#pragma once

#include <QColor>
#include <QObject>
#ifdef WIN32
    #include <Windows.h>
#endif
#include <GL/gl.h>

class TransferFunction : public QObject
{
    Q_OBJECT

public:
    TransferFunction();
    TransferFunction(int size);
    ~TransferFunction();

    void defaultTf(int type = 0, bool r = true, bool g = true, bool b = true, bool a = true);
    void smooth(bool r = true, bool g = true, bool b = true, bool a = true);

    void saveTo(QDataStream &out);
    void loadFrom(QDataStream &in);

    void setTf(QColor *colors);

    QColor* getAll();
    float *toData();


    QColor get(float intensity);
    void set(float intensity, QColor color);

    QColor get(int id);
    void set(int id, QColor color);

    int getSize();

private:
    int size;
    QColor *colors; // < components at normalized intensity

signals:
    void transFuncChangedAlpha();
    void transFuncChanged();


};
