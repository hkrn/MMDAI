/* Gouraud shading implementation for asset fragment shader */

uniform bool hasTexture;
uniform sampler2D mainTexture;
varying vec4 outColor;
varying vec2 outTexCoord;

void main() {
    vec4 color = outColor;
    if (hasTexture) {
        color *= texture2D(mainTexture, outTexCoord);
    }
    gl_FragColor = color;
}

