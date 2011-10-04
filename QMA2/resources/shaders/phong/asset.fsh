/* Phong shading implementation for asset fragment shader */

uniform vec4 lightColor;
uniform vec3 lightPosition;
uniform vec4 lightAmbient;
uniform vec4 lightDiffuse;
uniform vec4 lightSpecular;
uniform vec4 materialAmbient;
uniform vec4 materialDiffuse;
uniform vec4 materialEmission;
uniform vec4 materialSpecular;
uniform float materialShininess;
uniform float opacity;
uniform bool hasTexture;
uniform sampler2D mainTexture;
varying vec4 outColor;
varying vec4 outPosition;
varying vec4 outNormal;
varying vec2 outTexCoord;
const float kZero = 0.0;

void main() {
    vec3 normal = normalize(outNormal.xyz);
    vec3 light = normalize(lightPosition - outPosition.xyz);
    float diffuse = max(dot(light, normal), kZero);
    vec4 color = lightAmbient * materialAmbient + outColor + materialEmission;
    if (diffuse != kZero) {
        vec3 view = normalize(outPosition.xyz);
        vec3 halfway = normalize(light - view);
        float specular = pow(max(dot(normal, halfway), kZero), materialShininess);
        color += (lightDiffuse * materialDiffuse) * diffuse
               + (lightSpecular * materialSpecular) * specular;
    }
    if (hasTexture) {
        color *= texture2D(mainTexture, outTexCoord);
    }
    color.a = opacity;
    gl_FragColor = color;
}

