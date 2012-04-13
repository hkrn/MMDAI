#ifdef GL_ES
precision highp float;
#endif

varying vec4 outColor;

void main() {
    gl_FragColor = outColor;
}

