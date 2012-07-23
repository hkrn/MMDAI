/* gui/texture.fsh */
uniform sampler2D mainTexture;
varying vec2 outTexCoord;

void main() {
    vec4 color = texture2D(mainTexture, outTexCoord);
    gl_FragColor = color;
}

