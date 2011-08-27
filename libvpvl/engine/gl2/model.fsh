/* Phong shading implementation for fragment shader */

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
varying vec3 outPosition;
varying vec3 outNormal;
varying vec2 outMainTexCoord;
varying vec2 outSubTexCoord;
varying vec2 outToonTexCoord;
const float kZero = 0.0;

void main() {
    vec3 normal = normalize(outNormal);
    vec3 light = normalize(lightPosition - outPosition);
    float diffuse = max(dot(light, normal), kZero);
    vec4 color = lightAmbient + materialAmbient;
    if (diffuse != kZero) {
        vec3 view = normalize(outPosition);
        vec3 halfway = normalize(light - view);
        float specular = pow(max(dot(normal, halfway), kZero), materialShininess);
        color += lightDiffuse * materialDiffuse * diffuse + lightSpecular * materialSpecular * specular;
    }
    if (hasMainTexture) {
        if (isMainAdditive)
            color += texture2D(mainTexture, outMainTexCoord) + texture2D(toonTexture, outToonTexCoord);
        else
            color *= texture2D(mainTexture, outMainTexCoord) * texture2D(toonTexture, outToonTexCoord);
    }
    if (hasSubTexture) {
        if (isMainAdditive || isSubAdditive)
            color += texture2D(subTexture, outSubTexCoord) + texture2D(toonTexture, outToonTexCoord);
        else
            color *= texture2D(subTexture, outSubTexCoord) * texture2D(toonTexture, outToonTexCoord);
    }
    gl_FragColor = color;
}

