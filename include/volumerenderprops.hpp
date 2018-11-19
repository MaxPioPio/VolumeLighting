#pragma once

#include <QObject>
#include <QVector3D>
#include <QDataStream>

#include "transferfunction.hpp"

class VolumeRenderProps
        : public QObject
{
    Q_OBJECT
public:
    static const int DIRECT=0, MIP=1, ENTRY_POINTS=2, EXIT_POINTS=3, DEBUG_BOX=4;
    static const int NO_LIGHTING = 0, PHONG = 1, GLOBAL = 2, GLOBAL_PHONG = 3;

    VolumeRenderProps();

    float getStepSize();
    int getMode();
    int getLightingMode();
    int getShadowDimin();
    bool getLightDirectional();
    float getLightOpacityBaseStep();
    float getLightIntensity();
    float getLightBaseIntensity();
    float getLightSegmentLength();
    int getScatteringStepCount();
    float getScatteringRadius();

    // getter that return normalized values
    // (useful for updating gui slider positions)
    float getStepSizeN();
    float getLightPosXN();
    float getLightPosYN();
    float getLightPosZN();
    float getShadowDiminN();
    float getLightOpacityBaseStepN();
    float getLightIntensityN();
    float getLightBaseIntensityN();
    float getLightSegmentLengthN();
    float getScatteringStepCountN();
    float getScatteringRadiusN();

    void setTransFunc(TransferFunction *tf);
    TransferFunction* getTransFunc();

    QVector3D getLightPos();
    void setLightPos(QVector3D pos);
    void setShadowDimin(int v);

    void saveTo(QDataStream &out);
    void loadFrom(QDataStream &in);

private:
    int mode;
    int shadowDimin; // 1 = full size SV, 2 = half ...
    float stepSize;
    TransferFunction *transFunc;

    int lightingMode;
    QVector3D lightPos;
    float lightIntensity;
    float lightBaseIntensity;
    bool lightDirectional;
    float lightSegmentLength;
    float lightOpacityBaseStep;
    int scatteringStepCount;
    float scatteringRadius;

// SLOTS ----------------- //
public slots:
    void setMode(int mode);
    void setLightingMode(int mode);
    void setStepSize(float v);
    void setLightPosX(float v);
    void setLightPosY(float v);
    void setLightPosZ(float v);
    void setLightDirectional(bool v);
    void setLightOpacityBaseStep(float v);
    void setLightIntensity(float v);
    void setLightBaseIntensity(float v);
    void setLightSegmentLength(float v);
    void setScatteringStepCount(float v);
    void setScatteringRadius(float v);

private slots:
    void transFuncChangedSlot();
    void transFuncChangedAlphaSlot();

signals:
    void volumePropsChanged();
    void shadowPropsChanged();

};