#version 400

in vec3 modelPos;
out vec4 outColor;

void main() {
    outColor = vec4(modelPos, 1.f);
}
