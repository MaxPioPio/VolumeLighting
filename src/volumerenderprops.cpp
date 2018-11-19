#include "volumerenderprops.hpp"

#define STEP_SIZE_MIN 0.0002f
#define STEP_SIZE_MAX 0.1f
#define LIGHT_POS_MAX 10.f
#define LIGHT_BASE_OPAC_MIN 0.1f
#define LIGHT_BASE_OPAC_MAX 256.f
#define LIGHT_MAX_INTENSITY 10.f
#define LIGHT_MAX_BASE_INTENSITY 0.5f
#define LIGHT_MIN_SEGMENT_LENGTH 0.01f
#define LIGHT_MAX_SEGMENT_LENGTH 1.f
#define MIN_SCATTERING_RADIUS 0.f
#define MAX_SCATTERING_RADIUS 0.1f
#define MIN_SCATTERING_STEP_COUNT 2
#define MAX_SCATTERING_STEP_COUNT 6

VolumeRenderProps::VolumeRenderProps() {
    stepSize = (STEP_SIZE_MAX + STEP_SIZE_MIN)/2.f;
    mode = DIRECT;

    lightingMode = NO_LIGHTING;
    lightPos = QVector3D(0.f, 0.f, LIGHT_POS_MAX/2.f);
    shadowDimin = 1;
    lightDirectional = false;
    lightIntensity = 1.f;
    lightBaseIntensity = 0.f;
    lightSegmentLength = (LIGHT_MIN_SEGMENT_LENGTH + LIGHT_MAX_SEGMENT_LENGTH)/2.f;
    lightOpacityBaseStep = (LIGHT_BASE_OPAC_MIN + LIGHT_BASE_OPAC_MAX)/2.f;
    scatteringStepCount = MIN_SCATTERING_STEP_COUNT;
    scatteringRadius = MIN_SCATTERING_RADIUS;

    transFunc = new TransferFunction();
    connect(transFunc, SIGNAL(transFuncChangedAlpha()), this, SLOT(transFuncChangedAlphaSlot()));
    connect(transFunc, SIGNAL(transFuncChanged()), this, SLOT(transFuncChangedSlot()));

    emit shadowPropsChanged();
    emit volumePropsChanged();
}

void VolumeRenderProps::saveTo(QDataStream &out) {
    out << stepSize;
    out.operator <<(mode);
    out.operator <<(lightingMode);
    out << lightPos;
    out << shadowDimin;
    out << lightDirectional;
    out << lightIntensity;
    out << lightBaseIntensity;
    out << lightSegmentLength;
    out << lightOpacityBaseStep;
    out << scatteringRadius;
    out.operator <<(scatteringStepCount);
    transFunc->saveTo(out);
}

void VolumeRenderProps::loadFrom(QDataStream &in) {
    in >> stepSize;
    in.operator >>(mode);
    in.operator >>(lightingMode);
    in >> lightPos;
    in >> shadowDimin;
    in >> lightDirectional;
    in >> lightIntensity;
    in >> lightBaseIntensity;
    in >> lightSegmentLength;
    in >> lightOpacityBaseStep;
    in >> scatteringRadius;
    in.operator >>(scatteringStepCount);
    transFunc->loadFrom(in);
    emit shadowPropsChanged();
    emit volumePropsChanged();
}

float VolumeRenderProps::getStepSize() {
    return stepSize;
}

void VolumeRenderProps::setStepSize(float v) {
    stepSize = v * (STEP_SIZE_MAX - STEP_SIZE_MIN) + STEP_SIZE_MIN;
    emit volumePropsChanged();
}

void VolumeRenderProps::setShadowDimin(int v) {
    shadowDimin = v;
    emit shadowPropsChanged();
}

int VolumeRenderProps::getShadowDimin() {
    return shadowDimin;
}

int VolumeRenderProps::getMode() {
    return mode;
}

int VolumeRenderProps::getLightingMode() {
    return lightingMode;
}

void VolumeRenderProps::setMode(int i) {
    mode = i;
    emit volumePropsChanged();
}

void VolumeRenderProps::setLightingMode(int i) {
    lightingMode = i;
    emit volumePropsChanged();
}

void VolumeRenderProps::setTransFunc(TransferFunction *tf) {
    if(transFunc != nullptr)
        disconnect(transFunc, SIGNAL(transFuncChanged()), this, SLOT(transFuncChangedSlot()));
    this->transFunc = tf;
    disconnect(transFunc, SIGNAL(transFuncChanged()), this, SLOT(transFuncChangedSlot()));

    emit shadowPropsChanged();
    emit volumePropsChanged();
}

TransferFunction* VolumeRenderProps::getTransFunc() {
    return transFunc;
}

QVector3D VolumeRenderProps::getLightPos() {
    return lightPos;
}

void VolumeRenderProps::setLightPos(QVector3D pos) {
    lightPos = pos;
    emit shadowPropsChanged();
    emit volumePropsChanged();
}

float VolumeRenderProps::getLightIntensity() {
    return lightIntensity;
}

float VolumeRenderProps::getLightBaseIntensity() {
    return lightBaseIntensity;
}

float VolumeRenderProps::getLightSegmentLength() {
    return lightSegmentLength;
}

bool VolumeRenderProps::getLightDirectional() {
    return lightDirectional;
}

float VolumeRenderProps::getLightOpacityBaseStep() {
    return lightOpacityBaseStep;
}

int VolumeRenderProps::getScatteringStepCount() {
    return scatteringStepCount;
}

float VolumeRenderProps::getScatteringRadius() {
    return scatteringRadius;
}

/**** NORMALIZED GETTER ********************** */
float VolumeRenderProps::getStepSizeN() {
    return (stepSize - STEP_SIZE_MIN) / (STEP_SIZE_MAX - STEP_SIZE_MIN);
}

float VolumeRenderProps::getLightPosXN() {
    return (lightPos.x() + LIGHT_POS_MAX/2.f) / LIGHT_POS_MAX;
}

float VolumeRenderProps::getLightPosYN() {
    return (lightPos.y() + LIGHT_POS_MAX/2.f) / LIGHT_POS_MAX;
}

float VolumeRenderProps::getLightPosZN() {
    return (lightPos.z() + LIGHT_POS_MAX/2.f) / LIGHT_POS_MAX;
}

float VolumeRenderProps::getLightIntensityN() {
    return lightIntensity / LIGHT_MAX_INTENSITY;
}

float VolumeRenderProps::getLightBaseIntensityN() {
    return lightBaseIntensity / LIGHT_MAX_BASE_INTENSITY;
}

float VolumeRenderProps::getLightSegmentLengthN() {
    return (lightSegmentLength - LIGHT_MIN_SEGMENT_LENGTH) / (LIGHT_MAX_SEGMENT_LENGTH - LIGHT_MIN_SEGMENT_LENGTH);
}

float VolumeRenderProps::getLightOpacityBaseStepN() {
    return (lightOpacityBaseStep - LIGHT_BASE_OPAC_MIN) / (LIGHT_BASE_OPAC_MAX - LIGHT_BASE_OPAC_MIN);
}

float VolumeRenderProps::getScatteringStepCountN() {
    return (scatteringStepCount - MIN_SCATTERING_STEP_COUNT) / (MAX_SCATTERING_STEP_COUNT - MIN_SCATTERING_STEP_COUNT);
}

float VolumeRenderProps::getScatteringRadiusN() {
    return (scatteringRadius - MIN_SCATTERING_RADIUS) / (MAX_SCATTERING_RADIUS - MIN_SCATTERING_RADIUS);
}


/**** SLOTS ********************************** */

void VolumeRenderProps::transFuncChangedSlot() {
    emit volumePropsChanged();
}

void VolumeRenderProps::transFuncChangedAlphaSlot() {
    emit shadowPropsChanged();
}


void VolumeRenderProps::setLightPosX(float v) {
    lightPos.setX(v * LIGHT_POS_MAX - LIGHT_POS_MAX/2.f);
    emit shadowPropsChanged();
    emit volumePropsChanged();
}

void VolumeRenderProps::setLightPosY(float v) {
    lightPos.setY(v * LIGHT_POS_MAX - LIGHT_POS_MAX/2.f);
    emit shadowPropsChanged();
    emit volumePropsChanged();
}

void VolumeRenderProps::setLightPosZ(float v) {
    lightPos.setZ(v * LIGHT_POS_MAX - LIGHT_POS_MAX/2.f);
    emit shadowPropsChanged();
    emit volumePropsChanged();
}

void VolumeRenderProps::setLightIntensity(float v) {
    lightIntensity =  v * LIGHT_MAX_INTENSITY;
    emit shadowPropsChanged();
    emit volumePropsChanged();
}

void VolumeRenderProps::setLightBaseIntensity(float v) {
    lightBaseIntensity =  v * LIGHT_MAX_BASE_INTENSITY;
    emit volumePropsChanged();
}

void VolumeRenderProps::setLightSegmentLength(float v) {
    lightSegmentLength =  v * (LIGHT_MAX_SEGMENT_LENGTH - LIGHT_MIN_SEGMENT_LENGTH) + LIGHT_MIN_SEGMENT_LENGTH;
    emit shadowPropsChanged();
    emit volumePropsChanged();
}

void VolumeRenderProps::setLightDirectional(bool v) {
    lightDirectional = v;
    emit shadowPropsChanged();
    emit volumePropsChanged();
}

void VolumeRenderProps::setLightOpacityBaseStep(float v) {
    lightOpacityBaseStep = v * (LIGHT_BASE_OPAC_MAX - LIGHT_BASE_OPAC_MIN) + LIGHT_BASE_OPAC_MIN;
    emit shadowPropsChanged();
    emit volumePropsChanged();
}

void VolumeRenderProps::setScatteringStepCount(float v) {
    scatteringStepCount = v * (MAX_SCATTERING_STEP_COUNT - MIN_SCATTERING_STEP_COUNT) + MIN_SCATTERING_STEP_COUNT;
    emit shadowPropsChanged();
    emit volumePropsChanged();
}

void VolumeRenderProps::setScatteringRadius(float v) {
    scatteringRadius = v * (MAX_SCATTERING_RADIUS - MIN_SCATTERING_RADIUS) + MIN_SCATTERING_RADIUS;
    emit shadowPropsChanged();
    emit volumePropsChanged();
}
