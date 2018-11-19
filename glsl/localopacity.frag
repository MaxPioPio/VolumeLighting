#version 400

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

uniform vec3 lightPos;
uniform float baseStep = 128.f;
uniform float segmentLength = 0.05f;
uniform bool directional;

// applies the transfer function to the given normalized intensity value
// in [0;1]. The transfunc is stretched to fit over the actually occuring
// scalar data domain in the volume dataset given by VolumeProps.min/maxValue
vec4 transFunc(float intensity) {
    // fit the value to the relevant transfer function interval
    intensity = (intensity / (properties.maxValue - properties.minValue) + properties.minValue);
    // perform classification with the look up texture
    return texture(transferFunction, intensity);
}

void main(void)
{
//    outColor = vec4(1.f);//lightPos.x*5, vec3(0));//length(position)/10f, vec3(0.f));
//    //    outColor = vec4(mod((position.x + position.y + position.z)*400, 255)/255.f, vec3(0.f));
//    return;
    float stepLength = 3.f / (properties.width + properties.height + properties.depth);
    vec3 lightDir;
    if(directional)
        lightDir = normalize(lightPos);
    else
        lightDir = normalize(lightPos - position);

    vec3 pos;

    float curAlpha, alphaSum = 1.f;
    for(float t = 0.f; t < segmentLength; t += stepLength) {
        pos = position + t*lightDir;
        if(pos != clamp(pos, vec3(0.f), vec3(1.f))) {
            break;
        }
        curAlpha = 1.f - transFunc(texture(volumeData, pos).r).a;
        curAlpha = 1.f - pow(1.f - curAlpha, stepLength * baseStep);
        alphaSum *= curAlpha;
        if(alphaSum <= 0.f)
            break;
    }
    outColor = vec4(1.f - alphaSum, vec3(0.f));
}
