/* Gouraud shading implementation for model vertex shader */

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;
uniform vec4 lightColor;
uniform vec3 lightPosition;
uniform vec4 lightAmbient;
uniform vec4 lightDiffuse;
uniform vec4 lightSpecular;
uniform vec4 materialAmbient;
uniform vec4 materialDiffuse;
uniform vec4 materialSpecular;
uniform float materialShininess;
uniform float lightIntensity;
uniform bool isMainSphereMap;
uniform bool isSubSphereMap;
attribute vec4 inPosition;
attribute vec3 inNormal;
attribute vec2 inTexCoord;
attribute vec2 inToonTexCoord;
varying vec4 outColor;
varying vec2 outMainTexCoord;
varying vec2 outSubTexCoord;
varying vec2 outToonTexCoord;
const float kTwo = 2.0;
const float kOne = 1.0;
const float kHalf = 0.5;
const float kZero = 0.0;

vec2 makeSphereMap(vec4 position, vec3 normal) {
    vec3 R = reflect(position.xyz, normal);
    float M = kTwo * sqrt(R.x * R.x + R.y * R.y + (R.z + kOne) * (R.z + kOne));
    return vec2(R.x / M + kHalf, R.y / M + kHalf);
}

void main() {
    vec4 position = modelViewMatrix * inPosition;
    vec3 normal = normalize(normalMatrix * inNormal);
    vec3 light = normalize(lightPosition - position.xyz);
    float diffuse = max(dot(light, normal), kZero);
    vec4 color = (lightColor * lightIntensity * 2.0) * materialAmbient;
    if (diffuse != kZero) {
        vec3 view = normalize(position.xyz);
        vec3 halfway = normalize(light - view);
        float specular = pow(max(dot(normal, halfway), kZero), materialShininess);
        color += (kZero * lightDiffuse * materialDiffuse) * diffuse
              + (lightColor * lightSpecular * materialSpecular) * specular;
    }
    outColor = color;
    outMainTexCoord = isMainSphereMap ? makeSphereMap(position, normal) : inTexCoord;
    outSubTexCoord = isSubSphereMap ? makeSphereMap(position, normal) : inTexCoord;
    outToonTexCoord = inToonTexCoord;
    gl_Position = projectionMatrix * position;
}

