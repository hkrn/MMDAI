/* asset/model.fsh */
uniform bool hasMainTexture;
uniform bool hasSubTexture;
uniform bool isMainAdditive;
uniform bool isSubAdditive;
uniform bool hasDepthTexture;
uniform sampler2D mainTexture;
uniform sampler2D subTexture;
uniform sampler2D depthTexture;
uniform vec3 lightDirection;
uniform vec3 materialSpecular;
uniform float materialShininess;
uniform float opacity;
varying vec4 outColor;
varying vec4 outTexCoord;
varying vec4 outShadowCoord;
varying vec3 outEyeView;
varying vec3 outNormal;
const float kDepthThreshold = 0.00002;
const float kOne = 1.0;
const float kZero = 0.0;

float unpackDepth(const vec4 value) {
    const vec4 kBitShift = vec4(1.0 / 16777216.0, 1.0 / 65536.0, 1.0 / 256.0, 1.0);
    float depth = dot(value, kBitShift);
    return depth;
}

void main() {
    vec4 color = outColor;
    vec3 normal = normalize(outNormal);
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
    if (hasDepthTexture) {
        vec3 shadowCoord = outShadowCoord.xyz / outShadowCoord.w;
        vec4 depth4 = texture2D(depthTexture, shadowCoord.xy);
        float depth = unpackDepth(depth4) + kDepthThreshold;
        if (depth < shadowCoord.z)
            color.rgb *= 0.8;
    }
    vec3 halfVector = normalize(normalize(outEyeView) - lightDirection);
    float hdotn = max(dot(halfVector, outNormal), kZero);
    color.rgb += materialSpecular * pow(hdotn, max(materialShininess, kOne));
    color.a *= opacity;
    gl_FragColor = color;
}

