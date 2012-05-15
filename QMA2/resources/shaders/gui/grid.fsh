/* gui/grid.fsh */
#ifdef GL_ES
precision highp float;
#endif
varying vec3 outColor;

void main() {
    gl_FragColor = vec4(outColor, 1.0);
}

