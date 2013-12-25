/* gui/texture.fsh */
#if defined(GL_ES) || __VERSION__ >= 150
precision highp float;
#endif
#if __VERSION__ < 130
#define in varying
#define outPixelColor gl_FragColor
#else
out vec4 outPixelColor;
#endif
uniform sampler2D mainTexture;
in vec2 outTexCoord;

void main() {
    vec4 color = texture2D(mainTexture, outTexCoord);
    outPixelColor = color;
}

