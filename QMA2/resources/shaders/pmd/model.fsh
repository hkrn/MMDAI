/* pmd/model.fsh (unpackDepth and soft shadow based on three.js) */
#ifdef GL_ES
precision highp float;
#endif

uniform vec3 lightDirection;
uniform vec3 materialSpecular;
uniform vec2 depthTextureSize;
uniform float materialShininess;
uniform float opacity;
uniform bool useToon;
uniform bool useSoftShadow;
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
varying vec4 outShadowCoord;
varying vec3 outEyeView;
varying vec3 outNormal;
varying vec2 outToonCoord;
const float kOne = 1.0;
const float kZero = 0.0;
const vec4 kZero4 = vec4(kZero, kZero, kZero, kZero);

float unpackDepth(const vec4 value) {
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
    if (hasSubTexture) {
        if (isSubAdditive) {
            color.rgb += texture2D(subTexture, outTexCoord.zw).rgb;
        }
        else {
            color *= texture2D(subTexture, outTexCoord.zw);
        }
    }
    if (useToon) {
        vec3 toonColor = texture2D(toonTexture, outToonCoord).rgb;
        if (hasDepthTexture) {
            const vec2 kToonColorCoord = vec2(kZero, kOne);
            vec3 shadowCoord = outShadowCoord.xyz / outShadowCoord.w;
            if (useSoftShadow) {
                const float kSubPixel = 1.25;
                const float kDelta = kOne / 9.0;
                float depth = kZero, shadow = kZero;
                float xpoffset = kOne / depthTextureSize.x;
                float ypoffset = kOne / depthTextureSize.y;
                float dx0 = -kSubPixel * xpoffset;
                float dy0 = -kSubPixel * ypoffset;
                float dx1 = kSubPixel * xpoffset;
                float dy1 = kSubPixel * ypoffset;
                depth = unpackDepth(texture2D(depthTexture, shadowCoord.xy + vec2(dx0, dy0)));
                if (depth < shadowCoord.z) shadow += kDelta;
                depth = unpackDepth(texture2D(depthTexture, shadowCoord.xy + vec2(0.0, dy0)));
                if (depth < shadowCoord.z) shadow += kDelta;
                depth = unpackDepth(texture2D(depthTexture, shadowCoord.xy + vec2(dx1, dy0)));
                if (depth < shadowCoord.z) shadow += kDelta;
                depth = unpackDepth(texture2D(depthTexture, shadowCoord.xy + vec2(dx0, 0.0)));
                if (depth < shadowCoord.z) shadow += kDelta;
                depth = unpackDepth(texture2D(depthTexture, shadowCoord.xy));
                if (depth < shadowCoord.z) shadow += kDelta;
                depth = unpackDepth(texture2D(depthTexture, shadowCoord.xy + vec2(dx1, 0.0)));
                if (depth < shadowCoord.z) shadow += kDelta;
                depth = unpackDepth(texture2D(depthTexture, shadowCoord.xy + vec2(dx0, dy1)));
                if (depth < shadowCoord.z) shadow += kDelta;
                depth = unpackDepth(texture2D(depthTexture, shadowCoord.xy + vec2(0.0, dy1)));
                if (depth < shadowCoord.z) shadow += kDelta;
                depth = unpackDepth(texture2D(depthTexture, shadowCoord.xy + vec2(dx1, dy1)));
                if (depth < shadowCoord.z) shadow += kDelta;
                vec3 toon = texture2D(toonTexture, kToonColorCoord).rgb;
                color.rgb *= toon + (toonColor - toon) * (kOne - shadow);
            }
            else {
                const float kDepthThreshold = 0.00002;
                float depth = unpackDepth(texture2D(depthTexture, shadowCoord.xy)) + kDepthThreshold;
                if (depth < shadowCoord.z) {
                    vec3 toon = texture2D(toonTexture, kToonColorCoord).rgb;
                    color.rgb *= min(toonColor, toon);
                }
            }
        }
        else {
            color.rgb *= toonColor;
        }
    }
    vec3 halfVector = normalize(normalize(outEyeView) - lightDirection);
    float hdotn = max(dot(halfVector, normalize(outNormal)), kZero);
    color.rgb += materialSpecular * pow(hdotn, max(materialShininess, kOne));
    color.a *= opacity;
    gl_FragColor = color;
}

