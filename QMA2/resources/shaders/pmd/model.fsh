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
varying vec4 outShadowTexCoord;
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
    if (hasDepthTexture) {
#if 0
        float depth = texture2DProj(depthTexture, outShadowTexCoord).x;
        if ((outShadowTexCoord.z / outShadowTexCoord.w) >= depth)
            color.rgb *= 0.5;
#else
        vec4 divided = outPosition / outPosition.w;
        divided.z += 0.0005;
        float depth = texture2D(depthTexture, divided.st).z;
        if (outShadowTexCoord.w > 0.0 && depth < divided.z)
            color.rgb *= 0.5;
#endif
    }
    gl_FragColor = color;
}

