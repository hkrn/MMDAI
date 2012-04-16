uniform vec3 lightPosition;
uniform sampler2D mainTexture;
varying vec3 outPosition;
varying vec3 outNormal;
varying vec2 outTexCoord;
const vec4 kColor = vec4(0.65, 0.65, 0.65, 1.0);

void main() {
    vec3 position = normalize(outPosition);
    vec3 V = normalize(-position);
    vec3 N = normalize(outNormal);
    vec3 L = normalize(position - lightPosition);
    float diffuse = max(dot(L, N), 0.0);
    vec4 color = kColor + kColor * diffuse;
    vec4 textureColor = texture2D(mainTexture, outTexCoord);
#if 0
    gl_FragColor = color * textureColor;
#else
    gl_FragColor = textureColor;
#endif
}

