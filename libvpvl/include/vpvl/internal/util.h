#ifndef VPVL_INTERNAL_UTIL_H_
#define VPVL_INTERNAL_UTIL_H_

namespace vpvl
{

static inline void vectorN(char *&ptr, float *values, int n)
{
    for (int i = 0; i < n; i++) {
        values[i] = *reinterpret_cast<float *>(ptr);
        ptr += sizeof(float);
    }
}

static inline float spline1(const float t, const float p1, const float p2)
{
    return ((1 + 3 * p1 - 3 * p2) * t * t * t + (3 * p2 - 6 * p1) * t * t + 3 * p1 * t);
}

static inline float spline2(const float t, const float p1, const float p2)
{
    return ((3 + 9 * p1 - 9 * p2) * t * t + (6 * p2 - 12 * p1) * t + 3 * p1);
}

inline void vector3(char *&ptr, float *values)
{
    vectorN(ptr, values, 3);
}

inline void vector4(char *&ptr, float *values)
{
    vectorN(ptr, values, 4);
}

inline bool size8(char *&ptr, size_t &rest, size_t &size)
{
    if (sizeof(uint8_t) > rest)
        return false;
    size = *reinterpret_cast<uint8_t *>(ptr);
    ptr += sizeof(uint8_t);
    rest -= sizeof(uint8_t);
    return true;
}

inline bool size16(char *&ptr, size_t &rest, size_t &size)
{
    if (sizeof(uint16_t) > rest)
        return false;
    size = *reinterpret_cast<uint16_t *>(ptr);
    ptr += sizeof(uint16_t);
    rest -= sizeof(uint16_t);
    return true;
}

inline bool size32(char *&ptr, size_t &rest, size_t &size)
{
    if (sizeof(uint32_t) > rest)
        return false;
    size = *reinterpret_cast<uint32_t *>(ptr);
    ptr += sizeof(uint32_t);
    rest -= sizeof(uint32_t);
    return true;
}

inline bool validateSize(char *&ptr, size_t stride, size_t size, size_t &rest)
{
    size_t required = stride * size;
    if (required > rest)
        return false;
    ptr += required;
    rest -= required;
    return true;
}

inline void buildInterpolationTable(float x1, float x2, float y1, float y2, int size, float *&table)
{
    for (int i = 0; i < size; i++) {
        const float in = static_cast<double>(i) / size;
        float t = in;
        while (1) {
            const float v = spline1(t, x1, x2) - in;
            if (fabs(v) < 0.0001f)
                break;
            const float tt = spline2(t, x1, x2);
            if (tt == 0.0f)
                break;
            t -= v / tt;
        }
        table[i] = spline1(t, y1, y2);
    }
    table[size] = 1.0f;
}

}

#define VPVL_DISABLE_COPY_AND_ASSIGN(TypeName) \
TypeName(const TypeName &); \
         void operator=(const TypeName &);

#endif
