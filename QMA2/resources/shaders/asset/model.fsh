/* asset/model.fsh */
#ifdef GL_ES
precision highp float;
#endif

uniform bool hasMainTexture;
uniform float opacity;
uniform bool hasTexture;
uniform sampler2D mainTexture;
varying vec4 outColor;
varying vec2 outTexCoord;

void main() {
    vec4 color = outColor;
    if (hasTexture) {
        color *= texture2D(mainTexture, outTexCoord);
    }
    color *= opacity;
    gl_FragColor = color;
}

