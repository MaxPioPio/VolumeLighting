#version 400

layout(location = 0) in vec3 vertex;

out vec2 fragPos;

void main() {

    // positions have to fill [-1;1]x[-1;1]x[0]
    vec3 pos = vertex * 2.f;

    fragPos = (pos + vec3(1.f)).xy/2.f;
    gl_Position = vec4(pos, 1.f);
}


//#version 400

//layout(location = 0) in vec3 vertex;

//uniform mat4 modelViewMatrix;
//uniform mat4 projectionMatrix;

//out vec4 modelPos;

//void main() {
//    vec4 pos = projectionMatrix * modelViewMatrix * vec4(vertex, 1);

//    modelPos = vec4((vertex + vec3(1.f))/2.f, pos.w/255.f);
//    gl_Position = pos;
//}
