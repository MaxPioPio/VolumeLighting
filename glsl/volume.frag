#version 400

const float BASE_STEP = 200.f, OPACITY_TERMINATION = 1.f;

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

in vec2 fragPos;
out vec4 outColor;

//*********** UNIFORM START *************** //

// ---- rendering parameters ----- //
uniform vec3 lightPos;
uniform bool lightDirectional = true;
uniform vec3 eyePos;
uniform float baseLight = 0.f;
uniform int displayMode = 0;
uniform float step = 0.01f;
uniform bool phongLight = false;
uniform bool ambient = true;
uniform bool diffuse = true;
uniform bool specular = true;
uniform bool globalLight = false;

uniform VolumeProps properties;

// ---- Textures ----------------- //
uniform sampler3D volumeData;
uniform sampler1D transferFunction;

uniform sampler2D entryPoints;
uniform sampler2D exitPoints;

uniform sampler3D shadowVolume;

//*********** UNIFORM END ***************** //

// applies the transfer function to the given normalized intensity value
// in [0;1]. The transfunc is stretched to fit over the actually occuring
// scalar data domain in the volume dataset given by VolumeProps.min/maxValue
vec4 transFunc(float intensity) {
    // fit the value to the relevant transfer function interval
    intensity = (intensity / (properties.maxValue - properties.minValue) + properties.minValue);
    // perform classification with the look up texture
    return texture(transferFunction, intensity);
}

// calculates the gradient at samplePos with forward differences
vec3 gradient(vec3 samplePos) {
    float h = 3.f/(properties.width + properties.height + properties.depth);
    float x = texture(volumeData, samplePos + vec3(h, 0, 0)).r
            - texture(volumeData, samplePos - vec3(h, 0, 0)).r;
    float y = texture(volumeData, samplePos + vec3(0, h, 0)).r
            - texture(volumeData, samplePos - vec3(0, h, 0)).r;
    float z = texture(volumeData, samplePos + vec3(0, 0, h)).r
            - texture(volumeData, samplePos - vec3(0, 0, h)).r;
    return normalize(vec3(x,y,z));
}


// applies phong lighting to the sample at position samplePos
// with the given color and opacity. Uses the gradient a
// samplePos given by the gradient method as normal.
vec4 lighting(vec3 samplePos, vec3 color, float opacity) {

    // lightning parameters
    vec4 diffuseCol = vec4(color, 1.f);
    vec4 specularCol = vec4(0.2f);
    vec4 ambientCol = vec4(0.05f);
    const float intensity = 1, shininess = 10;

    // calculate the surface normal with the gradient
    vec3 normal = gradient(samplePos);

    // the eye vector (since we're in view space it's the position)
    vec3 toEye = normalize(eyePos - samplePos);

    // the normalized light vector
    vec3 toLight;
    if(lightDirectional)
        toLight = normalize(lightPos);
    else
        toLight = normalize(lightPos - samplePos);

    // the reflection vector
    vec3 reflect = reflect(toLight, normal);

    // calculate the resulting (diffuse and specular) optical properties
    vec4 diffuseSum = intensity * clamp(-dot(toLight, normal), 0.0, 1.0) * diffuseCol;
    vec4 specularSum = (shininess+2)/6.28318f * pow(clamp(-dot(toEye, reflect), 0.0, 1.0), shininess) * specularCol;

    vec4 result = vec4(0.f);
    if(ambient)
        result += ambientCol;
    if(diffuse)
        result += diffuseSum;
    if(specular)
        result += specularSum;

    return result;
}


vec4 directRendering(vec3 start, vec3 end) {

    if(start == end)
        return vec4(0.f);

    vec3 dir = (end - start);
    float t_end = length(dir);
    dir = normalize(dir);
    float diff = abs(step/t_end);// * (fract(sin(dir.z) * 43758.5453123)/2.f + 0.5f);

    float intensity;
    vec4 curCol;

    float alpha = 0, curOpacity;
    vec3 color = vec3(0.f);

    // iterative direct volume rendering sum:
    vec3 samplePos;
    bool cont = true;

    for(float t = 0; t <= t_end; t += diff) {   // diff vormals step

        // iterate along the ray
        samplePos = start + t*dir;

        // get the intensity value from the dataset
        intensity = texture(volumeData, samplePos).r;
        // apply the transfer function
        curCol = transFunc(intensity);

        if(curCol.a > 0.f) {
            // alpha correction for different step sizes
            curCol.a = 1.f - pow(1.f - curCol.a, step * BASE_STEP);
            curOpacity = (1.f - alpha) * curCol.a;

            // apply local lighting at current position for the resulting color
            if(phongLight)
                curCol.rgb = lighting(samplePos, curCol.rgb, curOpacity).rgb;

            // apply global lighting
            if(globalLight)
                curCol.rgb *= baseLight + (1.f - baseLight) * texture(shadowVolume, samplePos).r;

            // weight the color by the current opacity
            curCol.rgb *= curOpacity;

            // add the current color to the color sum
            color += curCol.rgb;
            // add the current alpha value to the opacity
            alpha += curOpacity;
        }

        // early ray termination
        if(alpha >= OPACITY_TERMINATION) {
            alpha = 1.f;
            break;
        }

    }
    return vec4(color, alpha);
}


vec4 maximumIntensity(vec3 start, vec3 end) {
    if(start == end)
        return vec4(0.f);

    vec3 dir = (end - start);
    float t_end = length(dir);
    dir = normalize(dir);
    float diff = abs(step/t_end);

    float intensity;
    float maxInt = -1.f;
    vec4 color = vec4(0.f);

    for(float t = 0; t <= t_end; t += diff) {
        intensity = texture(volumeData, start + t*dir).r;
        if(intensity > maxInt) {
            color = transFunc(intensity);
            maxInt = intensity;
        }
    }

    return color;
}

void main() {

    // obtain entry and exit points from the prerendered textures
    vec3 entryPoint = texture(entryPoints, fragPos).xyz;
    vec3 exitPoint = texture(exitPoints, fragPos).xyz;

    if(entryPoint == exitPoint) {
        discard;
    }

    // -------------- Render the volume with the given render mode ------------ //

    // volume rendering modes
    if(displayMode == 0)      // Direct rendering
        outColor = directRendering(entryPoint, exitPoint);
    else if(displayMode == 1) // Maximum Intensity Projection
        outColor = maximumIntensity(entryPoint, exitPoint);

    // Debug modes
    else if(displayMode == 2) // entry points
        outColor = vec4(entryPoint, 1.f);
    else if(displayMode == 3) // exit points
        outColor = vec4(exitPoint, 1.f);
    else if(displayMode == 4) // debug box (exit point box + default DVR)
        outColor = clamp(vec4(exitPoint, 0.f)*0.2f + directRendering(entryPoint, exitPoint), vec4(0), vec4(1));

    // incorrect mode
    else
        outColor = vec4(1.f, 0.f, 1.f, 1.f);

    outColor.rgb *= outColor.a;
}
