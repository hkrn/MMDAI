/* Phong shading implementation for vertex shader */

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform vec4 inColor;
attribute vec3 inPosition;
attribute vec3 inNormal;
varying vec3 outPosition;
varying vec3 outNormal;

void main() {
    // outPosition = vec3(modelViewMatrix * inPosition);
    // outNormal = normalize(transpose(inverse(modelViewMatrix)), inNormal);
    // gl_Position = projectionMatrix * modelViewMatrix * vec4(inPosition, 1);
    outPosition = vec3(gl_ModelViewMatrix * gl_Vertex);
    outNormal = normalize(gl_NormalMatrix * gl_Normal);
    gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;
}

