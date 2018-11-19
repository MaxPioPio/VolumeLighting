#version 400

#define PI 3.141509f

struct VolumeProps {
    int width;
    int height;
    int depth;
    float aspectX;
    float aspectY;
    float aspectZ;
    float minValue; // maximum normalized intensity value
    float maxValue; // (normalized: [0,1])
};

in vec3 position; // start position inside the volume of the segment
out vec4 outColor; // contains only an alpha value (to write to the alpha texture)

//********* UNIFORMS *************** //
uniform sampler3D volumeData;
uniform sampler1D transferFunction;
uniform VolumeProps properties;

uniform sampler3D localOpacity;
uniform sampler3D globalOpacity;

uniform float baseStep = 128.f;
uniform float theta;
uniform float phi;
uniform float radius = 0.05f;
uniform int stepCount = 4;

uniform vec3 lightPos;
uniform bool directional;
uniform float lightIntensity = 1.f;

// applies the transfer function to the given normalized intensity value
// in [0;1]. The transfunc is stretched to fit over the actually occuring
// scalar data domain in the volume dataset given by VolumeProps.min/maxValue
vec4 transFunc(float intensity) {
    // fit the value to the relevant transfer function interval
    intensity = (intensity / (properties.maxValue - properties.minValue) + properties.minValue);
    // perform classification with the look up texture
    return texture(transferFunction, intensity);
}

/// computes the light coming to position from direction dir
float raycast(vec3 position, vec3 dir) {
    float stepLength = 3.f / (properties.width + properties.height + properties.depth);
    float curAlpha;

    vec3 pos;
    float light = 0.f;
    for(float t = stepLength; t < radius; t += stepLength) {
        pos = position + t*dir;
        if(pos != clamp(pos, vec3(0.f), vec3(1.f))) {
            break;
        }
        curAlpha = 1.f - transFunc(texture(volumeData, pos).r).a;
        curAlpha = 1.f - pow(1.f - curAlpha, stepLength * baseStep);
        light += (1.f - texture(globalOpacity, pos).r) * curAlpha; // intensity * (1.f - tex..) = light from shadow map
    }
    return lightIntensity * light; // lightIntensity exists here to compute the actual light form shadow map implicitly
}

void main(void)
{
    vec3 dir = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
    outColor = vec4(raycast(position, dir)/(100.f*stepCount*stepCount));
    return;
}
