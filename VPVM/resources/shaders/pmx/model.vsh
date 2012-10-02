/* pmx/model.vsh */
#if __VERSION__ < 130
#define in attribute
#define out varying
#endif
//invariant gl_Position;
uniform mat4 modelViewProjectionMatrix;
uniform mat4 lightViewProjectionMatrix;
uniform mat3 normalMatrix;
uniform vec4 materialColor;
uniform vec3 cameraPosition;
uniform vec3 lightDirection;
uniform bool hasSphereTexture;
uniform bool hasDepthTexture;
in vec3 inDelta;
in vec4 inUVA0;
in vec4 inUVA1;
in vec3 inPosition;
in vec3 inNormal;
in vec2 inTexCoord;
out vec4 outColor;
out vec4 outTexCoord;
out vec4 outShadowCoord;
out vec4 outUVA1;
out vec3 outEyeView;
out vec3 outNormal;
const float kOne = 1.0;
const float kZero = 0.0;
const vec4 kOne4 = vec4(kOne, kOne, kOne, kOne);
const vec4 kZero4 = vec4(kZero, kZero, kZero, kZero);

vec2 makeSphereMap(const vec3 normal) {
    const float kHalf = 0.5;
    return vec2(normal.x * kHalf + kHalf, normal.y * kHalf + kHalf);
}

vec2 calculateToon(const vec3 normal) {
	return (vec3(1.0) + dot(lightDirection, -normal) * 0.5).xy;
}

void main() {
    vec4 position = vec4(inPosition + inDelta, kOne);
    vec3 normal = normalMatrix * inNormal;
    outEyeView = cameraPosition - inPosition;
    outNormal = inNormal;
    outColor = max(min(materialColor, kOne4), kZero4);
    outTexCoord.xy = inTexCoord + inUVA0.xy;
    outTexCoord.zw = hasSphereTexture ? makeSphereMap(normal) : calculateToon(normal);
    outUVA1 = inUVA1;
    if (hasDepthTexture) {
        outShadowCoord = lightViewProjectionMatrix * position;
    }
    gl_Position = modelViewProjectionMatrix * position;
}

