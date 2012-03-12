#define ENABLE_SHADOW 0
#ifdef GL_ES
precision highp float;
#define ENABLE_ALPHA_TEST 0
#else
#define ENABLE_ALPHA_TEST 1
#endif

uniform bool hasMainTexture;
uniform bool hasSubTexture;
uniform bool isMainAdditive;
uniform bool isSubAdditive;
uniform sampler2D mainTexture;
uniform sampler2D subTexture;
uniform sampler2D toonTexture;
varying vec4 outColor;
varying vec4 outTexCoord;
varying vec2 outToonTexCoord;
const float kAlphaThreshold = 0.05;
const float kOne = 1.0;
const float kZero = 0.0;
const vec4 kZero4 = vec4(kZero, kZero, kZero, kZero);

void main() {
    vec4 color = outColor;
    if (hasMainTexture) {
        if (isMainAdditive) {
            color.rgb += texture2D(mainTexture, outTexCoord.xy).rgb;
        }
        else {
            color *= texture2D(mainTexture, outTexCoord.xy);
        }
    }
    color *= texture2D(toonTexture, outToonTexCoord);
    if (hasSubTexture) {
        if (isSubAdditive) {
            color.rgb += texture2D(subTexture, outTexCoord.zw).rgb;
        }
        else {
            color *= texture2D(subTexture, outTexCoord.zw);
        }
    }
#if ENABLE_ALPHA_TEST
    if (color.a >= kAlphaThreshold)
        gl_FragColor = color;
    else
        discard;
#else
    gl_FragColor = color;
#endif
}

