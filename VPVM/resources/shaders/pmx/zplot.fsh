/* pmx/zplot.fsh */
#if __VERSION__ < 130
#define in varying
#define outPixelColor gl_FragColor
#else
out vec4 outPixelColor;
#endif
#ifdef GL_ES
precision highp float;
#endif
in vec4 outPosition;

void main() {
    outPixelColor = vec4(outPosition.z / outPosition.w);
}

