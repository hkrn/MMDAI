#ifndef VPVL_MATERIAL_H_
#define VPVL_MATERIAL_H_

#include <LinearMath/btVector3.h>
#include "vpvl/common.h"

namespace vpvl
{

class Material
{
public:
    Material();
    ~Material();

    static size_t stride(const char *data);

    void read(const char *data);

    const char *name() const {
        return m_name;
    }
    const btVector4 &ambient() const {
        return m_ambient;
    }
    const btVector4 &averageColor() const {
        return m_averageColor;
    }
    const btVector4 &diffuse() const {
        return m_diffuse;
    }
    const btVector4 &specular() const {
        return m_specular;
    }
    float alpha() const {
        return m_alpha;
    }
    float shiness() const {
        return m_shiness;
    }
    uint32_t countIndices() const {
        return m_nindices;
    }
    bool isEdgeEnabled() const {
        return m_edge;
    }

    void setName(const char *value) {
        vpvlStringCopySafe(m_name, value, sizeof(m_name));
    }
    void setAmbient(const btVector4 &value) {
        m_ambient = value;
    }
    void setAverageColor(const btVector4 &value) {
        m_averageColor = value;
    }
    void setDiffuse(const btVector4 &value) {
        m_diffuse = value;
    }
    void setSpecular(const btVector4 &value) {
        m_specular = value;
    }
    void setAlpha(float value) {
        m_alpha = value;
    }
    void setShiness(float value) {
        m_shiness = value;
    }
    void setEdgeEnabled(bool value) {
        m_edge = value;
    }

private:
    char m_name[20];
    btVector4 m_ambient;
    btVector4 m_averageColor;
    btVector4 m_diffuse;
    btVector4 m_specular;
    float m_alpha;
    float m_shiness;
    uint32_t m_nindices;
    uint8_t m_toonID;
    bool m_edge;
};

typedef btAlignedObjectArray<Material> MaterialList;

} /* namespace vpvl */

#endif
