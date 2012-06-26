/* pmx/model.fsh (unpackDepth and soft shadow based on three.js) */
uniform bool useToon;
uniform bool useSoftShadow;
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
uniform vec4 mainTextureBlend;
uniform vec4 sphereTextureBlend;
uniform vec4 toonTextureBlend;
uniform vec3 lightDirection;
uniform vec3 materialSpecular;
uniform vec2 depthTextureSize;
uniform float materialShininess;
uniform float opacity;
varying vec4 outColor;
varying vec4 outTexCoord;
varying vec4 outShadowCoord;
varying vec4 outUVA1;
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
    vec4 textureColor = vec4(1.0, 1.0, 1.0, 1.0);
    vec3 normal = normalize(outNormal);
    if (hasMainTexture) {
        float alpha = mainTextureBlend.a;
        textureColor = texture2D(mainTexture, outTexCoord.xy);
        textureColor.rgb *= mainTextureBlend.rgb * alpha + (1.0 - alpha);
    }
    color.rgb *= textureColor.rgb;
    if (hasSphereTexture) {
        if (isSPHTexture)
            textureColor.rgb *= texture2D(sphereTexture, outTexCoord.zw).rgb;
        else if (isSPATexture)
            textureColor.rgb += texture2D(sphereTexture, outTexCoord.zw).rgb;
        else if (isSubTexture)
            textureColor.rgb *= texture2D(sphereTexture, outUVA1.xy).rgb;
        textureColor.rgb *= sphereTextureBlend.rgb;
    }
    if (useToon) {
        if (hasToonTexture) {
            const vec2 kZero2 = vec2(kZero, kZero);
            vec4 toonColorRGBA = texture2D(toonTexture, kZero2);
            vec3 toonColor = toonColorRGBA.rgb * toonTextureBlend.rgb;
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
                const vec3 kOne3 = vec3(kOne, kOne, kOne);
                float lightNormal = dot(normal, -lightDirection);
                float w = max(min(lightNormal * 16.0 + 0.5, kOne), kZero);
                color.rgb *= toonColor + (kOne3 - toonColor) * w; 
            }
        }
    }
    vec3 halfVector = normalize(normalize(outEyeView) - lightDirection);
    float hdotn = max(dot(halfVector, normal), kZero);
    color.rgb += materialSpecular * pow(hdotn, max(materialShininess, kOne));
    color.a *= textureColor.a * opacity;
    gl_FragColor = color;
}

