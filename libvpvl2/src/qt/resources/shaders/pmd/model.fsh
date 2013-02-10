/* pmd/model.fsh (unpackDepth and soft shadow based on three.js) */
#if __VERSION__ < 130
#define in varying
#define outPixelColor gl_FragColor
#define texture(samp, uv) texture2D((samp), (uv))
#else
out vec4 outPixelColor;
#endif
#ifdef GL_ES
precision lowp float;
#endif
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
uniform vec4 materialColor;
uniform vec3 materialSpecular;
uniform vec3 lightDirection;
uniform vec2 depthTextureSize;
uniform float materialShininess;
uniform float opacity;
in vec4 outColor;
in vec4 outTexCoord;
in vec4 outShadowCoord;
in vec3 outEyeView;
in vec3 outNormal;
const float kOne = 1.0;
const float kZero = 0.0;
const vec4 kZero4 = vec4(kZero, kZero, kZero, kZero);

float unpackDepth(const vec4 value) {
    const vec4 kBitShift = vec4(1.0 / 16777216.0, 1.0 / 65536.0, 1.0 / 256.0, 1.0);
    float depth = dot(value, kBitShift);
    return depth;
}

vec4 applyTexture(vec4 color) {
    if (hasMainTexture) {
        if (isMainAdditive) {
            color.rgb += texture(mainTexture, outTexCoord.xy).rgb;
        }
        else {
            color *= texture(mainTexture, outTexCoord.xy);
        }
    }
    if (hasSubTexture) {
        if (isSubAdditive) {
            color.rgb += texture(subTexture, outTexCoord.zw).rgb;
        }
        else {
            color.rgb *= texture(subTexture, outTexCoord.zw).rgb;
        }
    }
    return color;
}

void main() {
    vec4 color = applyTexture(outColor);
    vec3 normal = normalize(outNormal);
    if (useToon) {
        const vec3 kOne3 = vec3(kOne, kOne, kOne);
        const vec2 kToonColorCoord = vec2(kZero, kOne);
        vec3 toonColor = texture(toonTexture, kToonColorCoord).rgb;
        float ldotn = dot(normal, -lightDirection);
        if (hasDepthTexture) {
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
                depth = unpackDepth(texture(depthTexture, shadowCoord.xy + vec2(dx0, dy0)));
                if (depth < shadowCoord.z) shadow += kDelta;
                depth = unpackDepth(texture(depthTexture, shadowCoord.xy + vec2(0.0, dy0)));
                if (depth < shadowCoord.z) shadow += kDelta;
                depth = unpackDepth(texture(depthTexture, shadowCoord.xy + vec2(dx1, dy0)));
                if (depth < shadowCoord.z) shadow += kDelta;
                depth = unpackDepth(texture(depthTexture, shadowCoord.xy + vec2(dx0, 0.0)));
                if (depth < shadowCoord.z) shadow += kDelta;
                depth = unpackDepth(texture(depthTexture, shadowCoord.xy));
                if (depth < shadowCoord.z) shadow += kDelta;
                depth = unpackDepth(texture(depthTexture, shadowCoord.xy + vec2(dx1, 0.0)));
                if (depth < shadowCoord.z) shadow += kDelta;
                depth = unpackDepth(texture(depthTexture, shadowCoord.xy + vec2(dx0, dy1)));
                if (depth < shadowCoord.z) shadow += kDelta;
                depth = unpackDepth(texture(depthTexture, shadowCoord.xy + vec2(0.0, dy1)));
                if (depth < shadowCoord.z) shadow += kDelta;
                depth = unpackDepth(texture(depthTexture, shadowCoord.xy + vec2(dx1, dy1)));
                if (depth < shadowCoord.z) shadow += kDelta;
                vec4 shadowColor = applyTexture(materialColor);
                shadowColor.rgb *= toonColor;
                color.rgb = shadowColor.rgb + (color.rgb - shadowColor.rgb) * (1.0 - shadow);
            }
            else {
                const float kDepthThreshold = 0.00002;
                float depth = unpackDepth(texture(depthTexture, shadowCoord.xy)) + kDepthThreshold;
                if (depth < shadowCoord.z) {
                    vec4 shadow = applyTexture(materialColor);
                    shadow.rgb *= toonColor;
                    color.rgb = shadow.rgb + (color.rgb - shadow.rgb) * (1.0 - depth);
                }
            }
        }
        else {
            float w = max(min(ldotn * 16.0 + 0.5, kOne), kZero);
            color.rgb *= toonColor + (kOne3 - toonColor) * w; 
        }
    }
    vec3 halfVector = normalize(normalize(outEyeView) - lightDirection);
    float hdotn = max(dot(halfVector, normal), kZero);
    color.rgb += materialSpecular * pow(hdotn, max(materialShininess, kOne));
    color.a *= opacity;
    outPixelColor = color;
}

