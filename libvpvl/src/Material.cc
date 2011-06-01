#include "vpvl/vpvl.h"
#include "vpvl/internal/util.h"

namespace vpvl
{

Material::Material()
    : m_ambient(0.0f, 0.0f, 0.0f, 1.0f),
      m_averageColor(0.0f, 0.0f, 0.0f, 1.0f),
      m_diffuse(0.0f, 0.0f, 0.0f, 1.0f),
      m_specular(0.0f, 0.0f, 0.0f, 1.0f),
      m_alpha(0.0f),
      m_shiness(0.0f),
      m_nindices(0),
      m_toonID(0),
      m_edge(false),
      m_firstSPH(false),
      m_firstSPA(false),
      m_secondSPH(false),
      m_secondSPA(false),
      m_private(0)
{
    memset(m_primaryTextureName, 0, sizeof(m_primaryTextureName));
    memset(m_secondTextureName, 0, sizeof(m_secondTextureName));
}

Material::~Material()
{
    memset(m_primaryTextureName, 0, sizeof(m_primaryTextureName));
    memset(m_secondTextureName, 0, sizeof(m_secondTextureName));
    m_ambient.setZero();
    m_averageColor.setZero();
    m_diffuse.setZero();
    m_specular.setZero();
    m_alpha = 0.0f;
    m_shiness = 0.0f;
    m_nindices = 0;
    m_toonID = 0;
    m_edge = false;
    m_firstSPH = false;
    m_firstSPA = false;
    m_secondSPH = false;
    m_secondSPA = false;
    m_private = 0;
}

size_t Material::stride(const char * /* data */)
{
    return sizeof(float) * 11 + (sizeof(uint8_t) * 2) + sizeof(uint32_t) + 20;
}

void Material::read(const char *data)
{
    char *ptr = const_cast<char *>(data);
    float ambient[3], diffuse[3], specular[3];
    vector3(ptr, diffuse);
    float alpha = *reinterpret_cast<float *>(ptr);
    ptr += sizeof(float);
    float shiness = *reinterpret_cast<float *>(ptr);
    ptr += sizeof(float);
    vector3(ptr, specular);
    vector3(ptr, ambient);
    uint8_t toonID = *reinterpret_cast<uint8_t *>(ptr);
    ptr += sizeof(uint8_t);
    uint8_t edge = *reinterpret_cast<uint8_t *>(ptr);
    ptr += sizeof(uint8_t);
    uint32_t nindices = *reinterpret_cast<uint32_t *>(ptr);
    ptr += sizeof(uint32_t);
    char name[20], *p;
    stringCopySafe(name, ptr, sizeof(name));
    stringCopySafe(m_rawName, ptr, sizeof(m_rawName));
    if ((p = strchr(name, '*')) != NULL) {
        *p = 0;
        stringCopySafe(m_primaryTextureName, name, sizeof(m_primaryTextureName));
        stringCopySafe(m_secondTextureName, p + 1, sizeof(m_secondTextureName));
        m_firstSPH = strstr(m_primaryTextureName, ".sph") != NULL;
        m_firstSPA = strstr(m_primaryTextureName, ".spa") != NULL;
        m_secondSPH = strstr(m_secondTextureName, ".sph") != NULL;
        m_secondSPA = strstr(m_secondTextureName, ".spa") != NULL;
    }
    else {
        stringCopySafe(m_primaryTextureName, name, sizeof(m_primaryTextureName));
        m_firstSPH = strstr(m_primaryTextureName, ".sph") != NULL;
        m_firstSPA = strstr(m_primaryTextureName, ".spa") != NULL;
    }

    m_ambient.setValue(ambient[0], ambient[1], ambient[2], 1.0f);
    m_diffuse.setValue(diffuse[0], diffuse[1], diffuse[2], 1.0f);
    m_specular.setValue(specular[0], specular[1], specular[2], 1.0f);
    btVector3 ac((m_diffuse + m_ambient) * 0.5f);
    m_averageColor.setValue(ac.x(), ac.y(), ac.z(), 1.0f);
    m_alpha = alpha;
    m_shiness = shiness;
    m_toonID = toonID == 0xff ? 0 : toonID + 1;
    m_edge = edge;
    m_nindices = nindices;
}

}
