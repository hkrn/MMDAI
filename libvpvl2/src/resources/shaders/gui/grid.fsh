/* gui/grid.fsh */
#if defined(GL_ES) || __VERSION__ >= 150
precision lowp float;
#endif
#if __VERSION__ < 130
#define in varying
#define outPixelColor gl_FragColor
#else
out vec4 outPixelColor;
#endif
in vec3 outColor;

void main() {
    outPixelColor = vec4(outColor, 1.0);
}

