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

#define VPVL_DISABLE_COPY_AND_ASSIGN(TypeName) \
TypeName(const TypeName &); \
         void operator=(const TypeName &);

}

#endif
