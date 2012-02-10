#ifdef GL_ES
precision highp float;
#endif

uniform bool hasMainTexture;
uniform bool hasToonTexture;
uniform bool hasSphereTexture;
uniform bool isSPHTexture;
uniform bool isSPATexture;
uniform bool isSubTexture;
uniform sampler2D mainTexture;
uniform sampler2D toonTexture;
uniform sampler2D sphereTexture;

varying vec4 outColor;
varying vec4 outTexCoord;
varying vec2 outToonTexCoord;
varying vec4 outUVA0;
varying vec4 outUVA1;
varying vec4 outUVA2;
varying vec4 outUVA3;
varying vec4 outUVA4;

const float kOne = 1.0;
const float kZero = 0.0;
const vec4 kZero4 = vec4(kZero, kZero, kZero, kZero);

void main() {
    vec4 color = outColor;
    if (hasMainTexture) {
        color *= texture2D(mainTexture, outTexCoord.xy);
    }
    if (hasSphereTexture) {
        if (isSPHTexture)
            color.rgb *= texture2D(sphereTexture, outTexCoord.zw).rgb;
        else if (isSPATexture)
            color.rgb *= texture2D(sphereTexture, outTexCoord.zw).rgb;
        else if (isSubTexture)
            color.rgb *= texture2D(sphereTexture, outUVA0.xy).rgb;
    }
    if (hasToonTexture) {
        color.rgb *= texture2D(toonTexture, outToonTexCoord).rgb;
    }
    gl_FragColor = color;
}

