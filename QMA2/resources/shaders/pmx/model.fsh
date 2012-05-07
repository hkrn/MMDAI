/* pmx/model.fsh (unpackDepth from three.js) */
#ifdef GL_ES
precision highp float;
#endif

uniform bool useToon;
uniform bool hasMainTexture;
uniform bool hasToonTexture;
uniform bool hasSphereTexture;
uniform bool hasDepthTexture;
uniform bool isSPHTexture;
uniform bool isSPATexture;
uniform bool isSubTexture;
uniform sampler2D mainTexture;
uniform sampler2D toonTexture;
uniform sampler2D sphereTexture;
uniform sampler2D depthTexture;
varying vec4 outColor;
varying vec4 outTexCoord;
varying vec4 outShadowCoord;
varying vec4 outUVA1;
varying vec2 outToonTexCoord;
const float kOne = 1.0;
const float kZero = 0.0;
const float kDepthThreshold = 0.00002;
const vec4 kZero4 = vec4(kZero, kZero, kZero, kZero);

float unpackDepth(const vec4 value) {
    const vec4 kBitShift = vec4(1.0 / 16777216.0, 1.0 / 65536.0, 1.0 / 256.0, 1.0);
    float depth = dot(value, kBitShift);
    return depth;
}

void main() {
    vec4 color = outColor;
    if (hasMainTexture) {
        color *= texture2D(mainTexture, outTexCoord.xy);
    }
    if (hasSphereTexture) {
        if (isSPHTexture)
            color.rgb *= texture2D(sphereTexture, outTexCoord.zw).rgb;
        else if (isSPATexture)
            color.rgb += texture2D(sphereTexture, outTexCoord.zw).rgb;
        else if (isSubTexture)
            color.rgb *= texture2D(sphereTexture, outUVA1.xy).rgb;
    }
    if (useToon) {
        if (hasToonTexture) {
            vec3 toonColor = texture2D(toonTexture, outToonTexCoord).rgb;
            if (hasDepthTexture) {
                vec3 shadowCoord = outShadowCoord.xyz / outShadowCoord.w;
                vec4 depth4 = texture2D(depthTexture, shadowCoord.xy);
                float depth = unpackDepth(depth4) + kDepthThreshold;
                if (depth < shadowCoord.z) {
                    vec3 toon = texture2D(toonTexture, vec2(0.0, 1.0)).rgb;
                    color.rgb *= min(toonColor, toon);
                }
            }
            else {
                color.rgb *= toonColor;
            }
        }
    }
    gl_FragColor = color;
}

