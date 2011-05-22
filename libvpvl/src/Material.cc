#include "vpvl/vpvl.h"

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
      m_edge(false)
{
}

Material::~Material()
{
    m_ambient.setZero();
    m_averageColor.setZero();
    m_diffuse.setZero();
    m_specular.setZero();
    m_alpha = 0.0f;
    m_shiness = 0.0f;
    m_nindices = 0;
    m_toonID = 0;
    m_edge = false;
}

size_t Material::stride(const char * /* data */)
{
    return sizeof(float) * 11 + (sizeof(uint8_t) * 2) + sizeof(uint32_t) + 20;
}

void Material::read(const char *data)
{
    char *ptr = const_cast<char *>(data);
    float ambient[3], diffuse[3], specular[3];
    vpvlStringGetVector3(ptr, diffuse);
    float alpha = *reinterpret_cast<float *>(ptr);
    ptr += sizeof(float);
    float shiness = *reinterpret_cast<float *>(ptr);
    ptr += sizeof(float);
    vpvlStringGetVector3(ptr, specular);
    vpvlStringGetVector3(ptr, ambient);
    uint8_t toonID = *reinterpret_cast<uint8_t *>(ptr);
    ptr += sizeof(uint8_t);
    uint8_t edge = *reinterpret_cast<uint8_t *>(ptr);
    ptr += sizeof(uint8_t);
    uint32_t nindices = *reinterpret_cast<uint32_t *>(ptr);
    ptr += sizeof(uint32_t);
    vpvlStringCopySafe(m_name, ptr, sizeof(m_name));
    m_ambient.setValue(ambient[0], ambient[1], ambient[2], 1.0f);
    m_diffuse.setValue(diffuse[0], diffuse[1], diffuse[2], 1.0f);
    m_specular.setValue(specular[0], specular[1], specular[2], 1.0f);
    m_alpha = alpha;
    m_shiness = shiness;
    m_toonID = toonID;
    m_edge = edge;
    m_nindices = nindices;
}

}
