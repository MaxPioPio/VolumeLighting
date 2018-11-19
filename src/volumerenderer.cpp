#include "volumerenderer.hpp"

#include "glutils.hpp"
#include "shadowrenderer.hpp"
#include <QImage>

static const QString entryExitVPath = "entryExit.vert", entryExitFPath = "entryExit.frag";
static const QString volumeVPath = "volume.vert", volumeFPath = "volume.frag";

VolumeRenderer::VolumeRenderer(QOpenGLWidget *renderWidget, VolumeData *volumeData, VolumeRenderProps *renderProps, int width, int height)
{
    shadowRenderer = nullptr;
    this->renderWidget = renderWidget;

    // store the dataset and the renderprops
    this->dataset = volumeData;
    connect(dataset, SIGNAL(dataChanged()), this, SLOT(datasetChanged()));
    this->renderProps = renderProps;

    volumeTexture = GL_INVALID_VALUE;
    transFunc = nullptr;
    transFuncTexture = GL_INVALID_VALUE;
    tfTexDirty = true;
    entryExitFBO = nullptr;

    // create the entry/exit points shader program ---------------------
    entryExitShaderProg = GLUtils::createShaderProg(entryExitVPath, entryExitFPath);

    // create the volume renering shader program -----------------------
    volumeShaderProg = GLUtils::createShaderProg(volumeVPath, volumeFPath);
    // set up the "constant" uniforms
    volumeShaderProg->bind();
    volumeShaderProg->setUniformValue("volumeSampler", 0);
    volumeShaderProg->setUniformValue("entryPoints", 1);
    volumeShaderProg->setUniformValue("exitPoints", 2);
    volumeShaderProg->setUniformValue("transferFunction", 3);
    volumeShaderProg->setUniformValue("shadowVolume", 4);
    volumeShaderProg->release();

    // create the FBOs through the resize method
    resizeCanvas(width, height);

    // create the shadow renderer
    shadowRenderer = new ShadowRenderer(this);
    shadowVolumeReady = true;
    updateShadow = false;
    // create the shadow update timer
    timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, SIGNAL(timeout()), this, SLOT(actualShadowUpdate()));

    // update the volume texture
    datasetChanged();
}

VolumeRenderer::~VolumeRenderer() {
    // delete the textures
    if(volumeTexture != GL_INVALID_VALUE)
        glDeleteTextures(1, &volumeTexture);
    if(transFuncTexture != GL_INVALID_VALUE)
        glDeleteTextures(1, &transFuncTexture);
}

void VolumeRenderer::resizeCanvas(int width, int height) {
    this->width = width;
    this->height = height;

    // generate new entry exit frame buffer objects
    entryExitFBO = new QOpenGLFramebufferObject(width, height, QOpenGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_RGB12);
    entryExitFBO->addColorAttachment(width, height);

    if(!entryExitFBO->isValid())
        qInfo() << this << "Volume Entry/Exit FBO not valid!";
}

void VolumeRenderer::updateTransFuncFrom(TransferFunction *tf) {
    // check if the transfer function changed
    if(transFunc != tf) {
        if(transFunc)
            disconnect(transFunc, SIGNAL(transFuncChanged()), this, SLOT(transFuncChanged()));
        transFunc = tf;
        connect(transFunc, SIGNAL(transFuncChanged()), this, SLOT(transFuncChanged()));
        tfTexDirty = true;
    }
    // update the transfer function texture if necessary
    if(tfTexDirty) {
        // clear errors
        QString err = GLUtils::glError();

        GLUtils::glFunc()->glActiveTexture(GL_TEXTURE0);

        // if no texture is created yet, generate one
        if(transFuncTexture == GL_INVALID_VALUE) {
            glGenTextures(1, &transFuncTexture);
            if(transFuncTexture == GL_INVALID_VALUE) {
                qWarning() << "Could not create transfer function texture!";
                return;
            }

            glBindTexture(GL_TEXTURE_1D, transFuncTexture);

            // set up parameters
            GLUtils::glFunc()->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            GLUtils::glFunc()->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            GLUtils::glFunc()->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            GLUtils::glFunc()->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            GLUtils::glFunc()->glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        }

        // upload the texture data to the target
        float *data = transFunc->toData();
        glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, transFunc->getSize(), 0, GL_RGBA, GL_FLOAT, data);
        delete[] data;

        err = GLUtils::glError();
        if(!err.isEmpty())
            qWarning() << "TF Texture Data: " << err;

        tfTexDirty = false;
    }
}

void VolumeRenderer::render(Camera *camera, PrimitiveUtils *primRenderer) {

    // clear the screen
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // if the render parameters are not complete, abort
    if(!dataset || !dataset->isReady()) {
        return;
    }

    // update the transfer function (texture) if changes were made
    updateTransFuncFrom(renderProps->getTransFunc());

    // update the shadow volume if necessary
    if(!shadowVolumeReady) {
        if(timer->isActive())
            timer->stop();
        shadowVolumeReady = shadowRenderer->updateShadowVolume(primRenderer);
    }

    // set the viewport
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);

    // Render the entry and exit points in the two fbo textures
    // using the "entryExit" shader program
    renderEntryExitPoints(camera, primRenderer);

    // render the volume using all the parameters and precalculated
    // textures and the "volume" shader program
    renderVolume(camera, primRenderer);

    if(!shadowVolumeReady)
        timer->start(SHADOW_UPDATE_DELAY);
}


/**
 * Renders the entry and exit points in the entryExitFBO color attachment 0 and 1
 * respectively of the volume as color coded rgb values (xyz in volume space).
 */
void VolumeRenderer::renderEntryExitPoints(Camera *camera, PrimitiveUtils *primRenderer)  {
    // clear errors
    QString err = GLUtils::glError();

    // setup the shader program and the fbo
    entryExitShaderProg->bind();
    entryExitFBO->bind();
    GLUtils::glFunc()->glEnableVertexAttribArray(0);

    entryExitShaderProg->setUniformValue("modelViewMatrix", *(camera->getViewMatrix()) * dataset->getNormalizeMatrix());
    entryExitShaderProg->setUniformValue("projectionMatrix", *(camera->getProjectionMatrix()));

    // Render Entry Points ------------------- //
    glCullFace(GL_BACK);
    GLUtils::glFunc()->glDrawBuffer(GL_COLOR_ATTACHMENT0); // entry texture
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    primRenderer->renderCube();

    // Render Exit Points
    glCullFace(GL_FRONT);
    GLUtils::glFunc()->glDrawBuffer(GL_COLOR_ATTACHMENT1); // exit texture
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    primRenderer->renderCube();

    // Release the Entry/Exit-program and all related objects
    GLUtils::glFunc()->glDisableVertexAttribArray(0);
    entryExitFBO->release();
    glFlush();
    entryExitShaderProg->release();

    err = GLUtils::glError();
    if(!err.isEmpty())
        qInfo() << "Volume entry/exit points render errors:" << err;
}


void VolumeRenderer::renderVolume(Camera *camera, PrimitiveUtils *primRenderer) {
    // clear errors
    QString err = GLUtils::glError();

    volumeShaderProg->bind();
    glCullFace(GL_BACK);

    // set the default buffer target
    GLUtils::glFunc()->glDrawBuffer(GL_FRONT_LEFT);
    err = GLUtils::glError();

    // set the volume rendering property uniforms
    volumeShaderProg->setUniformValue("displayMode", renderProps->getMode());
    volumeShaderProg->setUniformValue("step", renderProps->getStepSize());
    volumeShaderProg->setUniformValue("lightPos", renderProps->getLightPos());
    volumeShaderProg->setUniformValue("lightDirectional", renderProps->getLightDirectional());
    volumeShaderProg->setUniformValue("eyePos", camera->getEyePosition());
    volumeShaderProg->setUniformValue("baseLight", renderProps->getLightBaseIntensity());
    switch(renderProps->getLightingMode()) {
    case VolumeRenderProps::PHONG:
        volumeShaderProg->setUniformValue("phongLight", true);
        volumeShaderProg->setUniformValue("globalLight", false);
        break;
    case VolumeRenderProps::GLOBAL:
        volumeShaderProg->setUniformValue("phongLight", false);
        volumeShaderProg->setUniformValue("globalLight", true);
        break;
    case VolumeRenderProps::GLOBAL_PHONG:
        volumeShaderProg->setUniformValue("phongLight", true);
        volumeShaderProg->setUniformValue("globalLight", true);
        break;
    default:
        volumeShaderProg->setUniformValue("phongLight", false);
        volumeShaderProg->setUniformValue("globalLight", false);
    }

    // set the volume data property uniform
    volumeShaderProg->setUniformValue("properties.width", dataset->getProperties().width);
    volumeShaderProg->setUniformValue("properties.height", dataset->getProperties().height);
    volumeShaderProg->setUniformValue("properties.depth", dataset->getProperties().depth);
    volumeShaderProg->setUniformValue("properties.minValue", dataset->getProperties().minValue);
    volumeShaderProg->setUniformValue("properties.maxValue", dataset->getProperties().maxValue);

    // view matrix uniforms
    volumeShaderProg->setUniformValue("mvMat", *(camera->getViewMatrix()));
    volumeShaderProg->setUniformValue("normalMat", camera->getViewMatrix()->normalMatrix());

    err = GLUtils::glError();
    if(!err.isEmpty())
        qInfo() << "volume uniform errors:" << err;

    // bind the textures -------------------------------------------
    // bind the volume texture
    GLUtils::glFunc()->glActiveTexture(GL_TEXTURE0);
    GLUtils::glFunc()->glBindTexture(GL_TEXTURE_3D, volumeTexture);

    // entry exit points
    GLUtils::glFunc()->glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, entryExitFBO->textures().at(0));
    GLUtils::glFunc()->glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, entryExitFBO->textures().at(1));

    // transfer function
    GLUtils::glFunc()->glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_1D, transFuncTexture);

    // shadow volume
    GLUtils::glFunc()->glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_3D, shadowRenderer->getShadowTexture());

    err = GLUtils::glError();
    if(!err.isEmpty())
        qInfo() << "volume tex errors:" << err;

    // render the dummy plane to invoke the fragment shader program ----------------------------
    GLUtils::glFunc()->glEnableVertexAttribArray(0);
    primRenderer->renderPlaneXY();
    GLUtils::glFunc()->glDisableVertexAttribArray(0);

    volumeShaderProg->release();

    err = GLUtils::glError();
    if(!err.isEmpty())
        qInfo() << "volume final errors:" << err;
}



// **** SLOTS ****************************** //

void VolumeRenderer::datasetChanged() {
    QOpenGLFunctions_4_0_Core* glf = GLUtils::glFunc();

    if(!dataset->isReady())
        return;

    // delete the old texture if one exists
    if(volumeTexture != GL_INVALID_VALUE)
        glf->glDeleteTextures(1, &volumeTexture);

    // obtain a new texture from the dataset
    volumeTexture = dataset->createTexture();

    // update the shadow map
    shadowVolumeReady = false;
}

void VolumeRenderer::shadowPropsChanged() {
    shadowRenderer->shadowPropsChanged();
    if(!timer->isActive())
        timer->start(SHADOW_UPDATE_DELAY);
}

void VolumeRenderer::actualShadowUpdate() {
    shadowVolumeReady = false;
    renderWidget->update();
}

void VolumeRenderer::transFuncChanged() {
    tfTexDirty = true;
}
