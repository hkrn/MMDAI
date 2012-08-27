/* asset/model.vsh */
#if __VERSION__ < 130
#define in attribute
#define out varying
#endif
invariant gl_Position;
uniform mat4 modelMatrix;
uniform mat4 viewProjectionMatrix;
uniform mat4 lightViewProjectionMatrix;
uniform mat3 normalMatrix;
uniform vec4 materialDiffuse;
uniform vec3 materialColor;
uniform vec3 lightDirection;
uniform vec3 cameraPosition;
uniform bool isMainSphereMap;
uniform bool isSubSphereMap;
uniform bool hasDepthTexture;
in vec3 inPosition;
in vec3 inNormal;
in vec2 inTexCoord;
out vec4 outColor;
out vec4 outTexCoord;
out vec4 outShadowCoord;
out vec3 outEyeView;
out vec3 outNormal;
const float kOne = 1.0;
const float kZero = 0.0;
const vec4 kOne4 = vec4(kOne, kOne, kOne, kOne);
const vec4 kZero4 = vec4(kZero, kZero, kZero, kZero);

vec2 makeSphereMap(vec3 normal) {
    const float kHalf = 0.5;
    return vec2(normal.x * kHalf + kHalf, normal.y * -kHalf + kHalf);
}

void main() {
    vec4 position = modelMatrix * vec4(inPosition, kOne);
    vec4 normal4 = modelMatrix * vec4(inNormal, kZero);
    vec3 normal = normalize(normal4.xyz);
    float ldotn = max(dot(normal, -lightDirection), 0.0);
    vec4 color = vec4(materialColor + ldotn * materialDiffuse.rgb, materialDiffuse.a);
    outEyeView = cameraPosition - position.xyz;
    outNormal = normal;
    outColor = max(min(color, kOne4), kZero4);
    outTexCoord.xy = isMainSphereMap ? makeSphereMap(normal) : inTexCoord;
    outTexCoord.zw = isSubSphereMap ? makeSphereMap(normal) : inTexCoord;
    if (hasDepthTexture)
        outShadowCoord = lightViewProjectionMatrix * position;
    gl_Position = viewProjectionMatrix * position;
}

