/* pmx/edge.fsh */
#ifdef GL_ES
precision lowp float;
#endif

varying vec4 outColor;

void main() {
    gl_FragColor = outColor;
}

