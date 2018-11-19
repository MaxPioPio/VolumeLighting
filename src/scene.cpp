#include "scene.hpp"

#include <QStack>
#include <QFile>
#include <QFileInfo>

Scene::Scene()
{
    // volume rendering
    volume = new VolumeData();
    renderProps = new VolumeRenderProps();
}

Scene::~Scene() {
    delete volume;
    delete renderProps;
}

void Scene::update() {
}

void Scene::saveProject(QString path) {
    // open the file
    QFile file(path);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "Could not create file '" << path << "'' !";
        return;
    }

    QDataStream out(&file);
    out << volume->getFilePath();
    renderProps->saveTo(out);

    file.close();
}

void Scene::openProject(QString path) {       
    // check if the file exists
    QFile file(path);
    if(!file.exists()) {
        qWarning() << "File '" << path << "'' does not exist!";
        return;
    }
    // open the file
    if(!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open file '" << path << "'' !";
        return;
    }
    QDataStream in(&file);

    // volume data
    QString volumePath;
    in >> volumePath;
    // if the file at the absolute path saved in the project doesn't exist, try to laod it
    // from the 'default' volume data folder
    QFileInfo volumeFile = QFileInfo(volumePath);
    if(!volumeFile.exists()) {
        qInfo() << "Unable to load volume data from '" << volumePath << "'!";
        volumePath.remove(0, volumePath.lastIndexOf("/")+1);
        volumePath = "../VolumeData/" + volumePath;
        qInfo() << "Trying to load from '" << volumePath << "' instead.";
    }

    // load new volume data if needed
    if(volumePath != volume->getFilePath())
        volume->loadFrom(volumePath);
    // renderproperties
    renderProps->loadFrom(in);
    file.close();
}

// volume rendering
void Scene::loadVolume(QString path) {
    volume->loadFrom(path);
}

VolumeData* Scene::getVolume() {
    return volume;
}

VolumeRenderProps* Scene::getVolumeRenderProps() {
    return renderProps;
}
