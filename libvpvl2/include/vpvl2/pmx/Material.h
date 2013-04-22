/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2013  hkrn                                    */
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

#pragma once
#ifndef VPVL2_PMX_MATERIAL_H_
#define VPVL2_PMX_MATERIAL_H_

#include "vpvl2/IMaterial.h"
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

class VPVL2_API Material : public IMaterial
{
public:
    struct RGB3 {
        Color result;
        Vector3 base;
        Vector3 mul;
        Vector3 add;
        RGB3()
            : result(kZeroC),
              base(kZeroV3),
              mul(1, 1, 1),
              add(kZeroV3)
        {
        }
        void calculate() {
            const Vector3 &mixed = base * mul + add;
            result.setValue(mixed.x(), mixed.y(), mixed.z(), 1);
        }
        void calculateMulWeight(const Vector3 &value, const Scalar &weight) {
            static const Vector3 kOne3(1.0, 1.0, 1.0);
            mul = kOne3 - (kOne3 - value) * weight;
        }
        void calculateAddWeight(const Vector3 &value, const Scalar &weight) {
            add = value * weight;
        }
        void reset() {
            mul.setValue(1, 1, 1);
            add.setZero();
            calculate();
        }
    };
    struct RGBA3 {
        Color result;
        Color base;
        Color mul;
        Color add;
        RGBA3()
            : result(kZeroC),
              base(kZeroC),
              mul(1, 1, 1, 1),
              add(0, 0, 0, 0)
        {
        }
        void calculate() {
            const Vector3 &mixed = base * mul + add;
            const Scalar &alpha = base.w() * mul.w() + add.w();
            result.setValue(mixed.x(), mixed.y(), mixed.z(), alpha);
        }
        void calculateMulWeight(const Vector3 &value, const Scalar &weight) {
            static const Vector3 kOne3(1.0, 1.0, 1.0);
            const Vector3 &v = kOne3 - (kOne3 - value) * weight;
            mul.setValue(v.x(), v.y(), v.z(), 1.0f - (1.0f - value.w()) * weight);
        }
        void calculateAddWeight(const Vector3 &value, const Scalar &weight) {
            const Vector3 &v = value * weight;
            add.setValue(v.x(), v.y(), v.z(), value.w() * weight);
        }
        void reset() {
            mul.setValue(1, 1, 1, 1);
            add.setValue(0, 0, 0, 0);
            calculate();
        }
    };

    /**
     * Constructor
     */
    Material(IModel *modelRef);
    ~Material();

    static bool preparse(uint8_t *&data, size_t &rest, Model::DataInfo &info);
    static bool loadMaterials(const Array<Material *> &materials,
                              const Array<IString *> &textures,
                              int expectedIndices);
    static size_t estimateTotalSize(const Array<Material *> &materials, const Model::DataInfo &info);

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
    void mergeMorph(const Morph::Material *morph, const IMorph::WeightPrecision &weight);
    void resetMorph();

    IModel *parentModelRef() const { return m_modelRef; }
    const IString *name() const { return m_name; }
    const IString *englishName() const { return m_englishName; }
    const IString *userDataArea() const { return m_userDataArea; }
    const IString *mainTexture() const { return m_mainTextureRef; }
    const IString *sphereTexture() const { return m_sphereTextureRef; }
    const IString *toonTexture() const { return m_toonTextureRef; }
    SphereTextureRenderMode sphereTextureRenderMode() const { return m_sphereTextureRenderMode; }
    Color ambient() const { return m_ambient.result; }
    Color diffuse() const { return m_diffuse.result; }
    Color specular() const { return m_specular.result; }
    Color edgeColor() const { return m_edgeColor.result; }
    Color mainTextureBlend() const { return m_mainTextureBlend.result; }
    Color sphereTextureBlend() const { return m_sphereTextureBlend.result; }
    Color toonTextureBlend() const { return m_toonTextureBlend.result; }
    IndexRange indexRange() const { return m_indexRange; }
    float shininess() const { return m_shininess.x() * m_shininess.y() + m_shininess.z(); }
    float edgeSize() const { return m_edgeSize.x() * m_edgeSize.y() + m_edgeSize.z(); }
    int index() const { return m_index; }
    int textureIndex() const { return m_textureIndex; }
    int sphereTextureIndex() const { return m_sphereTextureIndex; }
    int toonTextureIndex() const { return m_toonTextureIndex; }
    bool isSharedToonTextureUsed() const { return m_useSharedToonTexture; }
    bool isCullFaceDisabled() const;
    bool hasShadow() const;
    bool isShadowMapDrawn() const;
    bool isSelfShadowDrawn() const;
    bool isEdgeDrawn() const;
    bool hasVertexColor() const;
    bool isPointDraw() const;
    bool isLineDraw() const;

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
    void setIndexRange(const IndexRange &value);
    void setShininess(float value);
    void setEdgeSize(float value);
    void setMainTextureIndex(int value);
    void setSphereTextureIndex(int value);
    void setToonTextureIndex(int value);
    void setFlags(int value);
    void setIndex(int value);

private:
    IModel *m_modelRef;
    IString *m_name;
    IString *m_englishName;
    IString *m_userDataArea;
    IString *m_mainTextureRef;
    IString *m_sphereTextureRef;
    IString *m_toonTextureRef;
    SphereTextureRenderMode m_sphereTextureRenderMode;
    RGB3 m_ambient;
    RGBA3 m_diffuse;
    RGB3 m_specular;
    RGBA3 m_edgeColor;
    RGBA3 m_mainTextureBlend;
    RGBA3 m_sphereTextureBlend;
    RGBA3 m_toonTextureBlend;
    IndexRange m_indexRange;
    Vector3 m_shininess;
    Vector3 m_edgeSize;
    int m_index;
    int m_textureIndex;
    int m_sphereTextureIndex;
    int m_toonTextureIndex;
    uint8_t m_flags;
    bool m_useSharedToonTexture;

    VPVL2_DISABLE_COPY_AND_ASSIGN(Material)
};

} /* namespace pmx */
} /* namespace vpvl2 */

#endif

