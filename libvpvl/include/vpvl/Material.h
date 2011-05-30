#ifndef VPVL_MATERIAL_H_
#define VPVL_MATERIAL_H_

#include <LinearMath/btVector3.h>
#include "vpvl/common.h"

namespace vpvl
{

typedef struct MaterialPrivate MaterialPrivate;

class Material
{
public:
    Material();
    ~Material();

    static size_t stride(const char *data);

    void read(const char *data);

    const char *primaryTextureName() const {
        return m_primaryTextureName;
    }
    const char *secondTextureName() const {
        return m_secondTextureName;
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
    uint8_t toonID() const {
        return m_toonID;
    }
    bool isEdgeEnabled() const {
        return m_edge;
    }
    bool isSpherePrimary() const {
        return m_firstSPH;
    }
    bool isSphereAuxPrimary() const {
        return m_firstSPA;
    }
    bool isSphereSecond() const {
        return m_secondSPH;
    }
    bool isSphereAuxSecond() const {
        return m_secondSPA;
    }
    MaterialPrivate *privateData() const {
        return m_private;
    }

    void setPrimaryTextureName(const char *value) {
        stringCopySafe(m_primaryTextureName, value, sizeof(m_primaryTextureName));
    }
    void setSecondTextureName(const char *value) {
        stringCopySafe(m_secondTextureName, value, sizeof(m_secondTextureName));
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
    void setPrivateData(MaterialPrivate *value) {
        m_private = value;
    }

private:
    char m_primaryTextureName[20];
    char m_secondTextureName[20];
    btVector4 m_ambient;
    btVector4 m_averageColor;
    btVector4 m_diffuse;
    btVector4 m_specular;
    float m_alpha;
    float m_shiness;
    uint32_t m_nindices;
    uint8_t m_toonID;
    bool m_edge;
    bool m_firstSPH;
    bool m_firstSPA;
    bool m_secondSPH;
    bool m_secondSPA;
    MaterialPrivate *m_private;
};

typedef btAlignedObjectArray<Material*> MaterialList;

} /* namespace vpvl */

#endif
