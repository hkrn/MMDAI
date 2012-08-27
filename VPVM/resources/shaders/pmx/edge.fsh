/* pmx/edge.fsh */
#if __VERSION__ < 130
#define in varying
#define outPixelColor gl_FragColor
#else
out vec4 outPixelColor;
#endif
#ifdef GL_ES
precision lowp float;
#endif

uniform float opacity;
in vec4 outColor;

void main() {
    vec4 color = outColor;
    color.a *= opacity;
    outPixelColor = color;
}

