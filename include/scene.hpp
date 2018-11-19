#pragma once

#include <QObject>

#include "volumedata.hpp"
#include "volumerenderprops.hpp"

class Scene : public QObject
{
    Q_OBJECT

public:
    Scene();
    ~Scene();

    void update();

    void openProject(QString path);
    void saveProject(QString path);

    // volume rendering
    void loadVolume(QString path);
    VolumeData* getVolume();
    VolumeRenderProps* getVolumeRenderProps();

private:

    // volume rendering
    VolumeData *volume;
    VolumeRenderProps *renderProps;
};