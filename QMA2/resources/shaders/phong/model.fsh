/* Phong shading implementation for model fragment shader */

uniform vec4 lightColor;
uniform vec3 lightPosition;
uniform vec4 lightAmbient;
uniform vec4 lightDiffuse;
uniform vec4 lightSpecular;
uniform vec4 materialAmbient;
uniform vec4 materialDiffuse;
uniform vec4 materialSpecular;
uniform float materialShininess;
uniform bool hasMainTexture;
uniform bool hasSubTexture;
uniform bool isMainAdditive;
uniform bool isSubAdditive;
uniform sampler2D mainTexture;
uniform sampler2D subTexture;
uniform sampler2D toonTexture;
varying vec4 outPosition;
varying vec4 outNormal;
varying vec2 outMainTexCoord;
varying vec2 outSubTexCoord;
varying vec2 outToonTexCoord;
const float kZero = 0.0;
const float kAlphaThreshold = 0.05;

// parameter adjustment (depends on toon shading)
const vec4 kAmbient = vec4(1.5, 1.5, 1.5, 1.0);
const vec4 kDiffuse = vec4(0.0, 0.0, 0.0, 1.0);
const vec4 kSpecular = vec4(0.75, 0.75, 0.75, 1.0);

void main() {
    vec3 normal = normalize(outNormal).xyz;
    vec3 light = normalize(lightPosition - outPosition.xyz);
    float diffuse = max(dot(light, normal), kZero);
    // vec4 color = kAmbient * lightAmbient * materialAmbient;
    vec4 color = lightAmbient * materialAmbient;
    if (diffuse != kZero) {
        vec3 view = normalize(outPosition.xyz);
        vec3 halfway = normalize(light - view);
        float specular = pow(max(dot(normal, halfway), kZero), materialShininess);
        // color += (kDiffuse * lightDiffuse * materialDiffuse) * diffuse
        //       + (kSpecular * lightSpecular * materialSpecular) * specular;
        color += (lightDiffuse * materialDiffuse) * diffuse
              + (lightSpecular * materialSpecular) * specular;
    }
    if (hasMainTexture) {
        if (isMainAdditive) {
            color += texture2D(mainTexture, outMainTexCoord) + texture2D(toonTexture, outToonTexCoord);
        }
        else {
            color *= texture2D(mainTexture, outMainTexCoord) * texture2D(toonTexture, outToonTexCoord);
        }
    }
    else {
        color *= texture2D(toonTexture, outToonTexCoord);
    }
    if (hasSubTexture) {
        if (isMainAdditive || isSubAdditive) {
            color += texture2D(subTexture, outSubTexCoord);
        }
        else {
            color *= texture2D(subTexture, outSubTexCoord);
        }
    }
    if (color.a >= kAlphaThreshold)
        gl_FragColor = color;
    else
        discard;
}

