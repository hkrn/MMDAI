/* Gourand shading implementation for model fragment shader */

uniform bool hasMainTexture;
uniform bool hasSubTexture;
uniform bool isMainAdditive;
uniform bool isSubAdditive;
uniform sampler2D mainTexture;
uniform sampler2D subTexture;
uniform sampler2D toonTexture;
varying vec4 outColor;
varying vec2 outMainTexCoord;
varying vec2 outSubTexCoord;
varying vec2 outToonTexCoord;
const float kAlphaThreshold = 0.05;

void main() {
    vec4 color = outColor;
    if (hasMainTexture) {
        if (isMainAdditive) {
            color += texture2D(mainTexture, outMainTexCoord) + texture2D(toonTexture, outToonTexCoord);
        }
        else {
            color *= texture2D(mainTexture, outMainTexCoord) * texture2D(toonTexture, outToonTexCoord);
        }
    }
    else {
        color *= texture2D(toonTexture, outToonTexCoord);
    }
    if (hasSubTexture) {
        if (isSubAdditive) {
            color += texture2D(subTexture, outSubTexCoord);
        }
        else {
            color *= texture2D(subTexture, outSubTexCoord);
        }
    }
    gl_FragColor = color;
}

