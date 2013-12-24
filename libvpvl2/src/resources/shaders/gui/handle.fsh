/* gui/handle.fsh */
#if defined(GL_ES) || __VERSION__ >= 150\n"
precision highp float;
#endif
#if __VERSION__ < 130
#define outPixelColor gl_FragColor
#else
out vec4 outPixelColor;
#endif
uniform vec4 color;

void main() {
    outPixelColor = color;
}

