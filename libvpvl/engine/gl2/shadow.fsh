/* Phong shading implementation for fragment shader */

uniform vec4 lightColor;
uniform vec3 lightPosition;
uniform vec4 lightAmbient;
uniform vec4 lightDiffuse;
uniform vec4 lightSpecular;
varying vec3 outPosition;
varying vec3 outNormal;
const float kZero = 0.0;

void main() {
    vec3 normal = normalize(outNormal);
    // vec3 light = normalize(lightPosition - outPosition);
    vec3 light = normalize(gl_LightSource[0].position.xyz - outPosition);
    float diffuse = max(dot(light, normal), kZero);
    // vec4 color = lightAmbient;
    vec4 color = gl_FrontLightProduct[0].ambient * gl_FrontMaterial.ambient;
    if (diffuse != kZero) {
        vec3 view = normalize(outPosition);
        vec3 halfway = normalize(light - view);
        // float specular = max(dot(normal, halfway), kZero);
        float specular = pow(max(dot(normal, halfway), kZero), gl_FrontMaterial.shininess);
        // color += lightDiffuse * diffuse + lightSpecular * specular;
        color += gl_FrontLightProduct[0].diffuse * gl_FrontMaterial.diffuse * diffuse
               + gl_FrontLightProduct[0].specular * gl_FrontMaterial.specular * specular;
    }
    gl_FragColor = color;
}

