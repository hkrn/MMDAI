/* gui/handle.fsh */
#if __VERSION__ < 130
#define outPixelColor gl_FragColor
#else
out vec4 outPixelColor;
#endif
uniform vec4 color;

void main() {
    outPixelColor = color;
}

