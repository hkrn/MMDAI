uniform vec3 lightColor;

void main() {
    vec4 color = vec4(lightColor, 1.0);
    gl_FragColor = color;
}

