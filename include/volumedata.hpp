#pragma once

#include <QObject>
#include <QString>
#include <QMatrix4x4>
#ifdef WIN32
    #include <Windows.h>
#endif
#include <GL/gl.h>

#include "transferfunction.hpp"

class Scene;
class RenderWidget;

struct VolumeDataProps {
    int width;
    int height;
    int depth;
    float aspectX;
    float aspectY;
    float aspectZ;
    float minValue; // maximum normalized intensity value
    float maxValue; // (normalized: [0,1])
};

class VolumeData : public QObject
{
    Q_OBJECT

public:
    VolumeData();

    void loadFrom(QString path);
    QString getFilePath();
    GLuint createTexture();

    bool isReady();
    char* getData();
    VolumeDataProps getProperties();
    QMatrix4x4 getNormalizeMatrix();

    float* createHistogram(int buckets);

private:
    VolumeDataProps properties;
    QMatrix4x4 normalizeMatrix;
    bool ready;

    QByteArray volumeData;
    short byteCount;

    float* histogram;
    int lastBuckets;

    QString filePath;

signals:
    void dataChanged();

};
