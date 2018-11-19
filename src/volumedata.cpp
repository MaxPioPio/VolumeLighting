#include "volumedata.hpp"

#include <QDataStream>
#include <QDebug>
#include <QFile>

#include <iostream>
#include <vector>
#include <fstream>

#include "renderwidget.hpp"
#include "glutils.hpp"

VolumeData::VolumeData()
{
    ready = false;
    // there is no histogram created
    lastBuckets = -1;
    histogram = nullptr;
}

void VolumeData::loadFrom(QString path) {

    // check if the file exists
    QFile file(path);
    if(!file.exists()) {
        qWarning() << "File '" << path << "'' does not exist!";
        return;
    }

    // open the file
    qInfo() << endl << "Loading volume from " << path;
    if(!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open file '" << path << "'' !";
        return;
    }

    filePath = path;

    // load resolution and apsect ratio from the first two lines
    int resX, resY, resZ;
    float aspectX, aspectY, aspectZ;

    QString strResolution = file.readLine();
    QString strAspect = file.readLine();
    QTextStream tsResolution(&strResolution, QIODevice::ReadOnly);
    QTextStream tsAspect(&strAspect, QIODevice::ReadOnly);
    tsResolution >> resX >> resY >> resZ;
    tsAspect >> aspectX >> aspectY >> aspectZ;

    // load the volume data as byte array
    volumeData = file.readAll();
    byteCount = volumeData.size()/(resX*resY*resZ);   // bytes per value

    // close the file
    file.close();

    // update properties
    properties.width = resX;
    properties.height = resY;
    properties.depth = resZ;
    properties.aspectX = aspectX;
    properties.aspectY = aspectY;
    properties.aspectZ = aspectZ;

    // determine max and min values of the data for normalizing
    // and correct the byte order
    uchar* d = reinterpret_cast<uchar*>(volumeData.data());
    int domain = pow(256, byteCount);
    int maxV = 0, minV = domain, v;
    int tmp, l, r;
    for(int i = 0; i < volumeData.size(); i += byteCount) {
        // determine current Value
        v = 0;
        for(int b=0; b < byteCount; b++)//byteCount-1; b>=0; b--)
            v = (v << 8) | static_cast<int>(d[i+b]);

        if(v > maxV)
            maxV = v;
        if(v < minV)
            minV = v;

        // fix the byte order to big endian and find max and min values
        for(int n = 0; n < (byteCount+1)/2; n++) {
            l = i + n;
            r = i + byteCount - 1 - n;
            tmp = volumeData[l];
            volumeData[l] = volumeData[r];
            volumeData[r] = tmp;
        }
    }
    // normalize the max and min intensity values
    properties.minValue = minV / (float) domain;
    properties.maxValue = maxV / (float) domain;

    // log properties
    qInfo() << "Dataset Dimension: " << resX << resY << resZ
            << " Aspect: " << aspectX << aspectY << aspectZ
            << " Bytes per value: " << byteCount
            << "Intensity values between " << properties.minValue << "and" << properties.maxValue;

    ready = true;

    histogram = nullptr;
    createHistogram(256);

    // calculate the normalize matrix
    float realWidth = properties.width * properties.aspectX;
    float realHeight = properties.height * properties.aspectY;
    float realDepth = properties.depth * properties.aspectZ;
    float max;
    if(realWidth > realHeight) {
        if(realWidth > realDepth)
            max = realWidth;
        else
            max = realDepth;
    } else {
        if(realHeight > realDepth)
            max = realHeight;
        else
            max = realDepth;
    }
    normalizeMatrix.setToIdentity();
    normalizeMatrix.scale(realWidth/max, realHeight/max, realDepth/max);

    emit dataChanged();
}

QString VolumeData::getFilePath() {
    return filePath;
}

/**
 * Creates a new 3D texture for this volume dataset. The caller has
 * to make sure that the texture is disposed correctly when it is no
 * longer used. This method uses texture unit 0. If the creation
 * fails, GL_INVALID_VALUE is returned, the name of the texture otherwise.
 *
 * @return the name of the created 3D volume texture
 */
GLuint VolumeData::createTexture() {
    if(!ready) {
        qWarning() << "Volume Data not ready! Unable to create texture.";
        return GL_INVALID_VALUE;
    }

    // clear errors
    QString err = GLUtils::glError();

    // set up the OpenGL texture ----------------------------------------------------
    QOpenGLFunctions_4_0_Core* glF = GLUtils::glFunc();

    // generate a texture and bind it
    GLuint texName = GL_INVALID_VALUE;
    GLUtils::glFunc()->glActiveTexture(GL_TEXTURE0);
    if(texName == GL_INVALID_VALUE)
        glGenTextures(1, &texName);
    if(texName == GL_INVALID_VALUE)
        qWarning() << "Texture name invalid!";
    glF->glBindTexture(GL_TEXTURE_3D, texName);

    // set up wrapping, filtering and pixel alignment
    glF->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glF->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glF->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glF->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glF->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glF->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // determine the data type
    GLenum dataType = GL_INVALID_ENUM;
    switch(byteCount) {
    case 1:
        dataType = GL_UNSIGNED_BYTE;
        break;
    case 2:
        dataType = GL_UNSIGNED_SHORT;
        break;
    case 4:
        dataType = GL_UNSIGNED_INT;
        break;
    default:
        qWarning() << "Can not upload volume data to texture. No suitable datatype for" <<
                      byteCount << "bytes per value. Using default type unsigned byte.";
        dataType = GL_UNSIGNED_BYTE;
        byteCount = 1;
        break;
    }

    // load the data to the GPU
    glF->glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, properties.width, properties.height, properties.depth,
                      0, GL_RED, dataType , volumeData.data());

    // unbind the texture
    glF->glBindTexture(GL_TEXTURE_3D, 0);

    // log GL errors
    err = GLUtils::glError();
    if(!err.isEmpty())
        qInfo() << "Volume Texture errors: " << err;

    return texName;
}

bool VolumeData::isReady() {
    return ready;
}

char* VolumeData::getData() {
    return volumeData.data();
}

VolumeDataProps VolumeData::getProperties() {
    return properties;
}

QMatrix4x4 VolumeData::getNormalizeMatrix() {
    return normalizeMatrix;
}

float* VolumeData::createHistogram(int buckets) {

    // if the histogram is up to date, just return it
    if(histogram != nullptr && lastBuckets == buckets)
        return histogram;

    // obtain the volume data array and max. possibly occuring value
    uchar* d = reinterpret_cast<uchar*>(volumeData.data());
    int domain = static_cast<int>(pow(256, byteCount));

    // create a new histogram array
    if(histogram != nullptr)
        delete[] histogram;
    histogram = new float[buckets];
    for(int i=0; i<buckets; i++)
        histogram[i] = 0;

    uint v;
    int bucketId, maxBucket=0;
    for(int i=0; i<volumeData.size(); i+=byteCount) {
        // get the value from the next byteCount bytes from d
        v = 0;
        for(int b=byteCount-1; b>=0; b--)
            v = (v << 8) | static_cast<int>(d[i+b]);

        // increase the bucket for this value
        bucketId = (v / (float) domain  - properties.minValue) / (properties.maxValue - properties.minValue) * buckets ;
        histogram[bucketId] += 1;

        // update max value
        if(histogram[bucketId] > maxBucket)
            maxBucket = histogram[bucketId];
    }

    // normalize the buckets (logarithmic)
    maxBucket = log(maxBucket);
    for(int i=0; i<buckets; i++)  {
        if(histogram[i] > 0)
        histogram[i] = log(histogram[i]) / maxBucket;
    }

    // save the number of buckets for this histogram so a new one
    // only has to be created if the bucket count changes
    lastBuckets = buckets;

    return histogram;
}
