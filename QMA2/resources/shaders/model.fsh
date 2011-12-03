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
uniform sampler2DShadow shadowTexture;
varying vec4 outColor;
varying vec4 outTexCoord;
varying vec4 outShadowTexCoord;
varying vec4 outDepth;
varying vec2 outToonTexCoord;
const float kAlphaThreshold = 0.05;
const float kOne = 1.0;
const float kZero = 0.0;
const vec4 kZero4 = vec4(kZero, kZero, kZero, kZero);

void main() {
#if ENABLE_SHADOW
    vec4 color = outDepth + (outColor - outDepth) * shadow2DProj(shadowTexture, outShadowTexCoord);
#else
    vec4 color = outColor;
#endif
    if (hasMainTexture) {
        if (isMainAdditive) {
            color += texture2D(mainTexture, outTexCoord.xy);
        }
        else {
            color *= texture2D(mainTexture, outTexCoord.xy);
        }
    }
    color *= texture2D(toonTexture, outToonTexCoord);
    if (hasSubTexture) {
        if (isSubAdditive) {
            color += texture2D(subTexture, outTexCoord.zw);
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

