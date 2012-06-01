/* gui/texture.fsh */
#ifdef GL_ES
precision highp float;
#endif
uniform sampler2D mainTexture;
varying vec2 outTexCoord;

void main() {
    vec4 color = texture2D(mainTexture, outTexCoord);
    gl_FragColor = color;
}

