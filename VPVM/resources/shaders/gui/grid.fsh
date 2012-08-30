/* gui/grid.fsh */
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

