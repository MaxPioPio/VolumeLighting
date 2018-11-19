#version 400

layout(location = 0) in vec3 vertex;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

out vec3 modelPos;

void main() {
    vec4 pos = projectionMatrix * modelViewMatrix * vec4(vertex, 1);

    modelPos = vertex + vec3(0.5f);
    gl_Position = pos;
}
