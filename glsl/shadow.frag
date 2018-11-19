#version 400

in vec3 position; // start position inside the volume of the segment
out vec4 outColor; // contains only an alpha value (to write to the alpha texture)

//********* UNIFORMS *************** //
uniform sampler3D localOpacity;
uniform sampler3D globalOpacity;
uniform float lightIntensity = 1.f;

uniform vec3 lightPos;
uniform float segmentLength = 0.05f;
uniform bool directional;


void main(void)
{
    vec3 s1; // first step along the light ray
    if(directional)
        s1 = position + segmentLength * normalize(lightPos);
    else
        s1 = position + segmentLength * normalize(lightPos - position);

    // light = I_0 * (1 - a_g(s1)) * (1 - a_lp(s0))
    outColor = vec4(lightIntensity * (1.f - texture(globalOpacity, s1).r) * (1.f - texture(localOpacity, position).r));
}
