/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2012  hkrn                                    */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAI project team nor the names of     */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#ifndef VPVL2_PMX_MATERIAL_H_
#define VPVL2_PMX_MATERIAL_H_

#include "vpvl2/pmx/Model.h"
#include "vpvl2/pmx/Morph.h"

namespace vpvl2
{
namespace pmx
{

/**
 * @file
 * @author hkrn
 *
 * @section DESCRIPTION
 *
 * Material class represents a morph of a Polygon Material Extended object.
 */

class VPVL2_API Material
{
public:
    enum SphereTextureRenderMode {
        kNone,
        kModulate,
        kAdditive,
        kSubTexture
    };
    struct Color3 {
        Color base;
        Vector3 mod;
        Vector3 add;
        Color3() : base(kZeroC), mod(1, 1, 1), add(kZeroV3) {}
        const Color calculate() const {
            const Vector3 &mixed = base * mod + add;
            return Color(mixed.x(), mixed.y(), mixed.z(), base.w());
        }
        void reset() {
            mod.setValue(1, 1, 1);
            add.setValue(0, 0, 0);
        }
    };

    /**
     * Constructor
     */
    Material();
    ~Material();

    static bool preparse(uint8_t *&data, size_t &rest, Model::DataInfo &info);
    static bool loadMaterials(const Array<Material *> &materials,
                              const Array<IString *> &textures,
                              int expectedIndices);

    /**
     * Read and parse the buffer with id and sets it's result to the class.
     *
     * @param data The buffer to read and parse
     * @param info Model information
     * @param size Size of vertex to be output
     */
    void read(const uint8_t *data, const Model::DataInfo &info, size_t &size);
    void write(uint8_t *data, const Model::DataInfo &info) const;
    size_t estimateSize(const Model::DataInfo &info) const;
    void mergeMorph(Morph::Material *morph, float weight);
    void resetMorph();

    const IString *name() const { return m_name; }
    const IString *englishName() const { return m_englishName; }
    const IString *userDataArea() const { return m_userDataArea; }
    const IString *mainTexture() const { return m_mainTexture; }
    const IString *sphereTexture() const { return m_sphereTexture; }
    const IString *toonTexture() const { return m_toonTexture; }
    SphereTextureRenderMode sphereTextureRenderMode() const { return m_sphereTextureRenderMode; }
    const Color ambient() const { return m_ambient.calculate(); }
    const Color diffuse() const { return m_diffuse.calculate(); }
    const Color specular() const { return m_specular.calculate(); }
    const Color edgeColor() const { return m_edgeColor.calculate(); }
    float shininess() const { return m_shininess.x() * m_shininess.y() + m_shininess.z(); }
    float edgeSize() const { return m_edgeSize.x() * m_edgeSize.y() + m_edgeSize.z(); }
    int textureIndex() const { return m_textureIndex; }
    int sphereTextureIndex() const { return m_sphereTextureIndex; }
    int toonTextureIndex() const { return m_toonTextureIndex; }
    int indices() const { return m_indices; }
    bool isSharedToonTextureUsed() const { return m_useSharedToonTexture; }
    bool isCullFaceDisabled() const { return m_flags & 0x01; }
    bool hasShadow() const { return m_flags & 0x02; }
    bool isShadowMapDrawn() const { return m_flags & 0x04; }
    bool isSelfShadowDrawn() const { return m_flags & 0x08; }
    bool isEdgeDrawn() const { return m_flags & 0x10; }

    void setName(const IString *value);
    void setEnglishName(const IString *value);
    void setUserDataArea(const IString *value);
    void setMainTexture(const IString *value);
    void setSphereTexture(const IString *value);
    void setToonTexture(const IString *value);
    void setSphereTextureRenderMode(SphereTextureRenderMode value);
    void setAmbient(const Color &value);
    void setDiffuse(const Color &value);
    void setSpecular(const Color &value);
    void setEdgeColor(const Color &value);
    void setShininess(float value);
    void setEdgeSize(float value);
    void setMainTextureIndex(int value);
    void setSphereTextureIndex(int value);
    void setToonTextureIndex(int value);
    void setIndices(int value);
    void setFlags(int value);

private:
    IString *m_name;
    IString *m_englishName;
    IString *m_userDataArea;
    IString *m_mainTexture;
    IString *m_sphereTexture;
    IString *m_toonTexture;
    SphereTextureRenderMode m_sphereTextureRenderMode;
    Color3 m_ambient;
    Color3 m_diffuse;
    Color3 m_specular;
    Color3 m_edgeColor;
    Vector3 m_shininess;
    Vector3 m_edgeSize;
    int m_textureIndex;
    int m_sphereTextureIndex;
    int m_toonTextureIndex;
    int m_indices;
    uint8_t m_flags;
    bool m_useSharedToonTexture;

    VPVL2_DISABLE_COPY_AND_ASSIGN(Material)
};

} /* namespace pmx */
} /* namespace vpvl2 */

#endif

