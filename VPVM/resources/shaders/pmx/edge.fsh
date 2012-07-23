/* pmx/edge.fsh */
#ifdef GL_ES
precision lowp float;
#endif

uniform float opacity;
varying vec4 outColor;

void main() {
    vec4 color = outColor;
    color.a *= opacity;
    gl_FragColor = color;
}

