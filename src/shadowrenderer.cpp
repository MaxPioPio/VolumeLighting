#include "shadowrenderer.hpp"

#include "glutils.hpp"

#define PI 3.141509f

static const QString localOpacVPath = "tex3d.vert", localOpacFPath = "localopacity.frag";
static const QString globalOpacVPath = "tex3d.vert", globalOpacFPath = "globalopacity.frag";
static const QString shadowVPath = "tex3d.vert", shadowFPath = "shadow.frag";
static const QString scatteringVPath = "tex3d.vert", scatteringFPath = "scattering.frag";

ShadowRenderer::ShadowRenderer(VolumeRenderer *volumeRenderer)
{    
    // clear errors
    QString err = GLUtils::glError();

    this->volumeRenderer = volumeRenderer;

    localOpacityTex = GL_INVALID_VALUE;
    globalOpacityTex = GL_INVALID_VALUE;
    shadowTex = GL_INVALID_VALUE;

    QOpenGLFunctions_4_0_Core *glF = GLUtils::glFunc();

    // create the shader programs and set the texture uniforms
    localProgram = GLUtils::createShaderProg(localOpacVPath, localOpacFPath);
    localProgram->bind();
    localProgram->setUniformValue("volumeData", 0);
    localProgram->setUniformValue("transferFunction", 1);
    localProgram->release();

    globalProgram = GLUtils::createShaderProg(globalOpacVPath, globalOpacFPath);
    globalProgram->bind();
    globalProgram->setUniformValue("localOpacity", 0);
    globalProgram->release();

    shadowProgram = GLUtils::createShaderProg(shadowVPath, shadowFPath);
    shadowProgram->bind();
    shadowProgram->setUniformValue("localOpacity", 0);
    shadowProgram->setUniformValue("globalOpacity", 1);
    shadowProgram->release();

    scatteringProgram = GLUtils::createShaderProg(scatteringVPath, scatteringFPath);
    scatteringProgram->bind();
    scatteringProgram->setUniformValue("volumeData", 0);
    scatteringProgram->setUniformValue("transferFunction", 1);
    scatteringProgram->setUniformValue("localOpacity", 2);
    scatteringProgram->setUniformValue("globalOpacity", 3);
    scatteringProgram->release();
    scatteringTheta = scatteringPhi = 0.f;

    // create the textures---------------------------------------------------------
    glF->glActiveTexture(GL_TEXTURE0);
    // local opacity texture
    glF->glGenTextures(1, &localOpacityTex);
    glF->glBindTexture(GL_TEXTURE_3D, localOpacityTex);
    glF->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glF->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glF->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glF->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glF->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // better approx. than LINEAR

    // global opacity texture
    glF->glGenTextures(1, &globalOpacityTex);
    glF->glBindTexture(GL_TEXTURE_3D, globalOpacityTex);
    glF->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glF->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glF->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glF->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glF->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // final shadow texture
    glF->glGenTextures(1, &shadowTex);
    glF->glBindTexture(GL_TEXTURE_3D, shadowTex);
    glF->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glF->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glF->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glF->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glF->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glF->glBindTexture(GL_TEXTURE_3D, 0);

    width = height = depth = -1.f;

    err = GLUtils::glError();
    if(!err.isEmpty())
        qWarning() << "Shadow Renderer Initialization 1: " << err;

    // create the FBOs
    glF->glGenFramebuffers(1, &localFBO);
    glF->glBindFramebuffer(GL_FRAMEBUFFER, localFBO);
    glF->glGenFramebuffers(1, &globalFBO);
    glF->glBindFramebuffer(GL_FRAMEBUFFER, globalFBO);
    glF->glGenFramebuffers(1, &shadowFBO);
    glF->glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);

    glF->glBindFramebuffer(GL_FRAMEBUFFER, 0);

    err = GLUtils::glError();
    if(!err.isEmpty())
        qWarning() << "Shadow Renderer Initialization 2: " << err;
}

void ShadowRenderer::shadowPropsChanged() {
    // to ensure that all shadow and opacity volumes are recomputed a
    // possible current scattering compuatation must be aborted.
    scatteringTheta = scatteringPhi = 0.f;
}

///
/// \brief updates the shadow volume if parameters were changed or otherweise
/// adds the next scattering iteration to the shadow/illumination volume
/// \param primRenderer the primitive renderer of the current OGL context
/// \return false if more scattering iterations are needed, false if all computations were completed
///
bool ShadowRenderer::updateShadowVolume(PrimitiveUtils *primRenderer) {
    updateBaseTextures();

    glViewport(0, 0, width, height);
    glCullFace(GL_BACK);
    glDisable(GL_DEPTH_TEST);

    // for the first step, the initial opacity and shadow volumes are computed
    if(scatteringTheta == 0.f && scatteringPhi == 0.f) {
        renderOpacities(primRenderer);
        renderShadowVolume(primRenderer);
    }
    // for all other steps the scattered light is added to the shadow volume
    if(volumeRenderer->renderProps->getScatteringRadius() > 0.f) {
        renderScattering(primRenderer);
        // compute the angle parameters for the next scattering iteration
        int stepCount = volumeRenderer->renderProps->getScatteringStepCount();
        scatteringPhi += PI/stepCount/(0.1f + 0.9f*sin(scatteringTheta));
        if(scatteringPhi >= 2.f * PI) {
            scatteringPhi = 0.f;
            scatteringTheta += PI/stepCount;
        }
        // as long as there are steps left, the shadowvolume is not ready
        // and needs more update iterations
        if(scatteringTheta <= PI)
            return false;
        else {
            scatteringTheta = scatteringPhi = 0.f;
            return true;
        }
    }
    return true;
}

void ShadowRenderer::updateBaseTextures() {
    if(!volumeRenderer->dataset->isReady())
        return;

    // clear errors
    QString err = GLUtils::glError();

    QOpenGLFunctions_4_0_Core *glF = GLUtils::glFunc();

    // the size of the textures is [width/dimin x height/dimin x depth/dimin]
    int dimin = volumeRenderer->renderProps->getShadowDimin();
    int _width = volumeRenderer->dataset->getProperties().width/dimin;
    int _height = volumeRenderer->dataset->getProperties().height/dimin;
    int _depth = volumeRenderer->dataset->getProperties().depth/dimin;

    if(_width == width && _height == height && _depth == depth) {
        return; // the textures already have the correct size
    }

    width = volumeRenderer->dataset->getProperties().width/dimin;
    height = volumeRenderer->dataset->getProperties().height/dimin;
    depth = volumeRenderer->dataset->getProperties().depth/dimin;

    glF->glActiveTexture(GL_TEXTURE0);
    glReadBuffer(GL_NONE);

    glF->glBindTexture(GL_TEXTURE_3D, localOpacityTex);
    glF->glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, width, height, depth, 0, GL_RED, GL_SHORT, NULL);

    glF->glBindTexture(GL_TEXTURE_3D, globalOpacityTex);
    glF->glTexImage3D(GL_TEXTURE_3D,0, GL_R8, width, height, depth, 0, GL_RED, GL_SHORT, NULL);

    glF->glBindTexture(GL_TEXTURE_3D, shadowTex);
    glF->glTexImage3D(GL_TEXTURE_3D,0, GL_R8, width, height, depth, 0, GL_RED, GL_SHORT, NULL);

    glF->glBindTexture(GL_TEXTURE_3D, 0);

    err = GLUtils::glError();
    if(!err.isEmpty())
        qWarning() << "Shadow Texture Creation: " << err;

    // attach the textures to the FBOs
    glF->glBindFramebuffer(GL_FRAMEBUFFER, localFBO);
    glF->glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, localOpacityTex, 0, 0);

    glF->glBindFramebuffer(GL_FRAMEBUFFER, globalFBO);
    glF->glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, globalOpacityTex, 0, 0);

    glF->glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glF->glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, shadowTex, 0, 0);

    glF->glBindFramebuffer(GL_FRAMEBUFFER, 0);

    err = GLUtils::glError();
    if(!err.isEmpty())
        qWarning() << "Shadow Texture Attachements: " << err;
}


void ShadowRenderer::process3DTexture(QOpenGLShaderProgram *program, GLuint fbo, GLuint texture, PrimitiveUtils *primRenderer, bool blend) {
    QOpenGLFunctions_4_0_Core *glF = GLUtils::glFunc();

    // enable blending if needed (scattering)
    if(blend) {
        glF->glEnable(GL_BLEND);
        glF->glBlendEquation(GL_FUNC_ADD);
        glF->glBlendFunc(GL_ONE, GL_ONE);
    }
    // setup the fbo
    glF->glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // setup the OpenGL state
    glF->glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glF->glEnableVertexAttribArray(0);

    // set the uniforms
    program->setUniformValue("layerCount", depth);

    // render [layer] = [depth] times a plane
    for(int i=0; i<depth; i++) {
        program->setUniformValue("layer", i);
        glF->glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0, i);
        primRenderer->renderPlaneXY();
    }

    // Release the program and the FBO
    glF->glDisableVertexAttribArray(0);
    glF->glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if(blend)
        glF->glDisable(GL_BLEND);

    glFlush();
}

void ShadowRenderer::renderOpacities(PrimitiveUtils *primRenderer) {
    // clear errors
    QString err = GLUtils::glError();

    QOpenGLFunctions_4_0_Core *glF = GLUtils::glFunc();
    VolumeDataProps dataProps = volumeRenderer->dataset->getProperties();
    VolumeRenderProps *renderProps = volumeRenderer->renderProps;

    // Render the Local Opacity Volume -----------------------------
    localProgram->bind();
    // bind the needed uniforms
    localProgram->setUniformValue("properties.width", dataProps.width);
    localProgram->setUniformValue("properties.height", dataProps.height);
    localProgram->setUniformValue("properties.depth", dataProps.depth);
    localProgram->setUniformValue("properties.minValue", dataProps.minValue);
    localProgram->setUniformValue("properties.maxValue", dataProps.maxValue);

    localProgram->setUniformValue("lightPos", renderProps->getLightPos());
    localProgram->setUniformValue("baseStep", renderProps->getLightOpacityBaseStep());
    localProgram->setUniformValue("directional", renderProps->getLightDirectional());
    localProgram->setUniformValue("segmentLength", renderProps->getLightSegmentLength());

    // bind the textures
    glF->glActiveTexture(GL_TEXTURE0);
    glF->glBindTexture(GL_TEXTURE_3D, volumeRenderer->volumeTexture);
    glF->glActiveTexture(GL_TEXTURE1);
    glF->glBindTexture(GL_TEXTURE_1D, volumeRenderer->transFuncTexture);

    // render the result to the 3D texture localOpac
    process3DTexture(localProgram, localFBO, localOpacityTex, primRenderer);
    localProgram->release();

    err = GLUtils::glError();
    if(!err.isEmpty())
        qInfo() << "Local Opacity Render Errors:" << err;

    // Render the Global Opacity Volume ----------------------------
    globalProgram->bind();
    // bind the needed texture uniforms
    globalProgram->setUniformValue("properties.width", dataProps.width);
    globalProgram->setUniformValue("properties.height", dataProps.height);
    globalProgram->setUniformValue("properties.depth", dataProps.depth);
    globalProgram->setUniformValue("properties.minValue", dataProps.minValue);
    globalProgram->setUniformValue("properties.maxValue", dataProps.maxValue);
    globalProgram->setUniformValue("lightPos", renderProps->getLightPos());
    globalProgram->setUniformValue("baseStep", renderProps->getLightOpacityBaseStep());
    globalProgram->setUniformValue("directional", renderProps->getLightDirectional());
    globalProgram->setUniformValue("segmentLength", renderProps->getLightSegmentLength());

    // bind the textures
    glF->glActiveTexture(GL_TEXTURE0);
    glF->glBindTexture(GL_TEXTURE_3D, localOpacityTex);

    // render the result to the 3D texture globalOpac
    process3DTexture(globalProgram, globalFBO, globalOpacityTex, primRenderer);
    globalProgram->release();

    err = GLUtils::glError();
    if(!err.isEmpty())
        qInfo() << "Global Opacity Render Errors:" << err;
}

void ShadowRenderer::renderShadowVolume(PrimitiveUtils *primRenderer) {
    // clear errors
    QString err = GLUtils::glError();

    QOpenGLFunctions_4_0_Core *glF = GLUtils::glFunc();

    shadowProgram->bind();
    // bind the needed uniforms
    VolumeRenderProps *renderProps = volumeRenderer->renderProps;
    shadowProgram->setUniformValue("lightPos", renderProps->getLightPos());
    shadowProgram->setUniformValue("directional", renderProps->getLightDirectional());
    shadowProgram->setUniformValue("lightIntensity", renderProps->getLightIntensity());
    shadowProgram->setUniformValue("segmentLength", renderProps->getLightSegmentLength());

    // bind the textures
    glF->glActiveTexture(GL_TEXTURE0);
    glF->glBindTexture(GL_TEXTURE_3D, localOpacityTex);
    glF->glActiveTexture(GL_TEXTURE1);
    glF->glBindTexture(GL_TEXTURE_3D, globalOpacityTex);

    // render the result to the 3D shadow/lighting texture
    process3DTexture(shadowProgram, shadowFBO, shadowTex, primRenderer);
    shadowProgram->release();

    err = GLUtils::glError();
    if(!err.isEmpty())
        qInfo() << "Shadow Volume Render Errors:" << err;
}

void ShadowRenderer::renderScattering(PrimitiveUtils *primRenderer) {
    // clear errors
    QString err = GLUtils::glError();

    QOpenGLFunctions_4_0_Core *glF = GLUtils::glFunc();

    scatteringProgram->bind();
    // bind the needed uniforms
    VolumeDataProps dataProps = volumeRenderer->dataset->getProperties();
    VolumeRenderProps *renderProps = volumeRenderer->renderProps;

    // Render the Local Opacity Volume -----------------------------
    // bind the needed uniforms
    scatteringProgram->setUniformValue("properties.width", dataProps.width);
    scatteringProgram->setUniformValue("properties.height", dataProps.height);
    scatteringProgram->setUniformValue("properties.depth", dataProps.depth);
    scatteringProgram->setUniformValue("properties.minValue", dataProps.minValue);
    scatteringProgram->setUniformValue("properties.maxValue", dataProps.maxValue);

    scatteringProgram->setUniformValue("lightPos", renderProps->getLightPos());
    scatteringProgram->setUniformValue("baseStep", renderProps->getLightOpacityBaseStep());
    scatteringProgram->setUniformValue("directional", renderProps->getLightDirectional());
    scatteringProgram->setUniformValue("lightIntensity", renderProps->getLightIntensity());
    scatteringProgram->setUniformValue("stepCount", renderProps->getScatteringStepCount());
    scatteringProgram->setUniformValue("theta", scatteringTheta);
    scatteringProgram->setUniformValue("phi", scatteringPhi);
    scatteringProgram->setUniformValue("radius", renderProps->getScatteringRadius());
    //shadowProgram->setUniformValue("stepCount", renderProps->?);

    // bind the textures
    glF->glActiveTexture(GL_TEXTURE0);
    glF->glBindTexture(GL_TEXTURE_3D, volumeRenderer->volumeTexture);
    glF->glActiveTexture(GL_TEXTURE1);
    glF->glBindTexture(GL_TEXTURE_1D, volumeRenderer->transFuncTexture);
    glF->glActiveTexture(GL_TEXTURE2);
    glF->glBindTexture(GL_TEXTURE_3D, localOpacityTex);
    glF->glActiveTexture(GL_TEXTURE3);
    glF->glBindTexture(GL_TEXTURE_3D, globalOpacityTex);

    // add the result to the 3D shadow/lighting texture with bleding
    process3DTexture(scatteringProgram, shadowFBO, shadowTex, primRenderer, true); // true = additive blending!
    scatteringProgram->release();

    err = GLUtils::glError();
    if(!err.isEmpty())
        qInfo() << "Scattering Render Errors:" << err;
}

GLuint ShadowRenderer::getShadowTexture() {
    return shadowTex;
}
