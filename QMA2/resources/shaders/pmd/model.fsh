/* pmd/model.fsh (unpackDepth from three.js) */
#ifdef GL_ES
precision highp float;
#endif

uniform bool hasMainTexture;
uniform bool hasSubTexture;
uniform bool hasDepthTexture;
uniform bool isMainAdditive;
uniform bool isSubAdditive;
uniform sampler2D mainTexture;
uniform sampler2D subTexture;
uniform sampler2D toonTexture;
uniform sampler2D depthTexture;
varying vec4 outColor;
varying vec4 outTexCoord;
varying vec4 outPosition;
varying vec4 outShadowCoord;
varying vec2 outToonTexCoord;
const float kAlphaThreshold = 0.05;
const float kOne = 1.0;
const float kZero = 0.0;
const vec4 kZero4 = vec4(kZero, kZero, kZero, kZero);

float unpackDepth(vec4 value) {
    const vec4 kBitShift = vec4(1.0 / 16777216.0, 1.0 / 65536.0, 1.0 / 256.0, 1.0);
    float depth = dot(value, kBitShift);
    return depth;
}

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
    if (hasDepthTexture) {
        vec3 shadowCoord = outShadowCoord.xyz / outShadowCoord.w;
        vec4 depth4 = texture2D(depthTexture, outShadowCoord.xy);
        float depth = depth4.z; // unpackDepth(depth4);
        if (depth < shadowCoord.z)
            color.r = 1.0;
    }
    gl_FragColor = color;
}

