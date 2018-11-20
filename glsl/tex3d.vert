#version 400

layout(location = 0) in vec3 vertex;

out vec3 position; // position inside the volume

uniform int layer;
uniform int layerCount;

void main() {

    // fragment positions have to fill [-1;1]x[-1;1]x[0]
    vec3 pos = vertex * 2.f;
    gl_Position = vec4(pos, 1.f);

    // position inside the volume
    position = vertex + vec3(0.5f);
    position.z = float(layer)/layerCount;
}
