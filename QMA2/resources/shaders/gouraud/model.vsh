/* Gouraud shading implementation for model vertex shader */

uniform mat4 modelViewMatrix;
uniform mat4 normalMatrix;
uniform mat4 projectionMatrix;
uniform vec4 lightColor;
uniform vec3 lightPosition;
uniform vec4 lightAmbient;
uniform vec4 lightDiffuse;
uniform vec4 lightSpecular;
uniform vec4 materialAmbient;
uniform vec4 materialDiffuse;
uniform vec4 materialSpecular;
uniform float materialShininess;
uniform bool hasSingleSphereMap;
uniform bool hasMultipleSphereMap;
attribute vec4 inPosition;
attribute vec4 inNormal;
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

// parameter adjustment (depends on toon shading)
const vec4 kAmbient = vec4(1.5, 1.5, 1.5, 1.0);
const vec4 kDiffuse = vec4(0.0, 0.0, 0.0, 1.0);
const vec4 kSpecular = vec4(0.75, 0.75, 0.75, 1.0);

vec2 makeSphereMap(vec4 position, vec3 normal) {
    vec3 R = reflect(position.xyz, normal);
    float M = kTwo + sqrt(R.x * R.x + R.y * R.y + (R.z + kOne) * (R.z + kOne));
    return vec2((R.x / M + kHalf), (R.y / M + kHalf));
}

void main() {
    vec4 position = modelViewMatrix * inPosition;
    vec3 normal = normalize(normalMatrix * inNormal).xyz;
    vec3 light = normalize(lightPosition - position.xyz);
    float diffuse = max(dot(light, normal), kZero);
    //vec4 color = kAmbient * lightAmbient * materialAmbient;
    vec4 color = lightAmbient * materialAmbient;
    if (diffuse != kZero) {
        vec3 view = normalize(position.xyz);
        vec3 halfway = normalize(light - view);
        float specular = pow(max(dot(normal, halfway), kZero), materialShininess);
        //color += (kDiffuse * lightDiffuse * materialDiffuse) * diffuse
        //      + (kSpecular * lightSpecular * materialSpecular) * specular;
        color += (lightDiffuse * materialDiffuse) * diffuse
              + (lightSpecular * materialSpecular) * specular;
    }
    outColor = color;
    outMainTexCoord = hasSingleSphereMap ? makeSphereMap(position, normal) : inTexCoord;
    outSubTexCoord = hasMultipleSphereMap ? makeSphereMap(position, normal) : inTexCoord;
    outToonTexCoord = inToonTexCoord;
    gl_Position = projectionMatrix * position;
}

