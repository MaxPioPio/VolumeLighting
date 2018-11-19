#pragma once

#include <QOpenGLShaderProgram>
#include <QOpenGLFramebufferObject>

#include "volumerenderer.hpp"
#include "volumerenderprops.hpp"
#include "primitives.hpp"

/**
* The ShadowRenderer calculates the loccal and global opacity volumes
* and the resulting shadow volume that is used in the final render
* process for the global lighting effects for the volume. The ShadowRenderer
* is a friend class of the VolumeRenderer to allow it to access the volume data
* and textures.
* The ShadowRenderer is only used in the render methods of the  VolumeRenderer.
*/
class ShadowRenderer {
public:
    ShadowRenderer(VolumeRenderer *volumeRenderer);
    void shadowPropsChanged();
    bool updateShadowVolume(PrimitiveUtils *primRenderer);
    GLuint getShadowTexture();

private:
    // creates empty textures with the correct size
    void updateBaseTextures();
    // calculates the local and global opacity textures
    void renderOpacities(PrimitiveUtils *primRenderer);
    // renders the resulting shadow volume
    void renderShadowVolume(PrimitiveUtils *primRenderer);
    // blends the light contribution from single scattering effects to the shadow volume
    void renderScattering(PrimitiveUtils *primRenderer);
    // processes the 3D texture in the given FBO. The correct shader program has to be bound beforehand
    void process3DTexture(QOpenGLShaderProgram *program, GLuint fbo, GLuint texture, PrimitiveUtils *primRenderer, bool blend = false);

    int width, height, depth;
    GLuint localOpacityTex, globalOpacityTex, shadowTex;
    GLuint localFBO, globalFBO, shadowFBO;

    QOpenGLShaderProgram *localProgram, *globalProgram, *shadowProgram, *scatteringProgram;
    float scatteringTheta, scatteringPhi;
    VolumeRenderer *volumeRenderer;

};