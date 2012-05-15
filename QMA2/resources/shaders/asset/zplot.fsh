/* asset/zplot.fsh based on three.js */
#ifdef GL_ES
precision highp float;
#endif
const vec4 kBitShift = vec4(16777216.0, 65536.0, 256.0, 1.0);
const vec4 kBitMask = vec4(0, 1.0 / 256.0, 1.0 / 256.0, 1.0 / 256.0);

vec4 packDepth(const float depth) {
    vec4 res = fract(depth * kBitShift);
    res -= res.xxyz * kBitMask;
    return res;
}

void main() {
    gl_FragColor = packDepth(gl_FragCoord.z);
}

