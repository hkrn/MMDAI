uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform vec3 lightPosition;
attribute vec4 inPosition;

void main() {
    vec4 position = inPosition;
    vec3 normal = normalize(inNormal);
    vec3 light = normalize(lightPosition - position.xyz);
    gl_Position = projectionMatrix * modelViewMatrix * position;
}

