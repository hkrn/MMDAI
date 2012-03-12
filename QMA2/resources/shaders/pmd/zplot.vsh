uniform mat4 modelViewProjectionMatrix;
attribute vec4 inPosition;

void main() {
    vec4 position = modelViewProjectionMatrix * inPosition;
    gl_Position = position;
}

