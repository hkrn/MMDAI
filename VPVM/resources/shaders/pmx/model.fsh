/* pmx/model.fsh (unpackDepth and soft shadow based on three.js) */
#if __VERSION__ < 130
#define in varying
#define outPixelColor gl_FragColor
#define texture(samp, uv) texture2D((samp), (uv))
#else
out vec4 outPixelColor;
#endif
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
uniform vec4 materialColor;
uniform vec3 materialSpecular;
uniform vec3 lightDirection;
uniform vec2 depthTextureSize;
uniform float materialShininess;
uniform float opacity;
in vec4 outColor;
in vec4 outTexCoord;
in vec4 outShadowCoord;
in vec4 outUVA1;
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

vec4 applyTexture(const vec4 color) {
    vec4 textureColor = vec4(1.0, 1.0, 1.0, 1.0);
    if (hasMainTexture) {
        float alpha = mainTextureBlend.a;
        textureColor = texture(mainTexture, outTexCoord.xy);
        textureColor.rgb *= mainTextureBlend.rgb * alpha + (1.0 - alpha);
    }
    if (hasSphereTexture) {
        if (isSPHTexture)
            textureColor.rgb *= texture(sphereTexture, outTexCoord.zw).rgb;
        else if (isSPATexture)
            textureColor.rgb += texture(sphereTexture, outTexCoord.zw).rgb;
        else if (isSubTexture)
            textureColor.rgb *= texture(sphereTexture, outUVA1.xy).rgb;
        textureColor.rgb *= sphereTextureBlend.rgb;
    }
    return color * textureColor;
}

void main() {
    vec4 color = applyTexture(outColor);
    vec3 normal = normalize(outNormal);
    if (useToon) {
        if (hasToonTexture) {
            const vec2 kZero2 = vec2(kZero, kZero);
            vec4 toonColorRGBA = texture(toonTexture, kZero2);
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
    color.a *= opacity;
	outPixelColor = color;
}

