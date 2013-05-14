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

#ifndef VPVL2_PMD2_MATERIAL_H_
#define VPVL2_PMD2_MATERIAL_H_

#include "vpvl2/Common.h"
#include "vpvl2/IMaterial.h"
#include "vpvl2/pmd2/Model.h"

namespace vpvl2
{

namespace pmd2
{

class VPVL2_API Material : public IMaterial
{
public:
    static const int kNameSize = 20;

    Material(IModel *parentModelRef, IEncoding *encodingRef);
    ~Material();

    IModel *parentModelRef() const;
    const IString *name() const;
    const IString *englishName() const;
    const IString *userDataArea() const;
    const IString *mainTexture() const;
    const IString *sphereTexture() const;
    const IString *toonTexture() const;
    SphereTextureRenderMode sphereTextureRenderMode() const;
    Color ambient() const;
    Color diffuse() const;
    Color specular() const;
    Color edgeColor() const;
    Color mainTextureBlend() const;
    Color sphereTextureBlend() const;
    Color toonTextureBlend() const;
    IndexRange indexRange() const;
    float shininess() const;
    IVertex::EdgeSizePrecision edgeSize() const;
    int index() const;
    int textureIndex() const;
    int sphereTextureIndex() const;
    int toonTextureIndex() const;
    bool isSharedToonTextureUsed() const;
    bool isCullingDisabled() const;
    bool hasShadow() const;
    bool hasShadowMap() const;
    bool isSelfShadowEnabled() const;
    bool isEdgeEnabled() const;

    void setName(const IString * /* value */) {}
    void setEnglishName(const IString * /* value */) {}
    void setUserDataArea(const IString * /* value */) {}
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
    void setEdgeSize(const IVertex::EdgeSizePrecision & /* value */) {}
    void setMainTextureIndex(int /* value */) {}
    void setSphereTextureIndex(int /* value */) {}
    void setToonTextureIndex(int /* value */) {}
    void setFlags(int /* value */) {}

    static bool preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info);
    static bool loadMaterials(const PointerArray<Material> &materials,
                              const PointerArray<IString> &textures,
                              int expectedIndices);
    static void writeMaterials(const Array<Material *> &materials, const Model::DataInfo &info, uint8_t *&data);
    static size_t estimateTotalSize(const Array<Material *> &materials, const Model::DataInfo &info);

    void read(const uint8_t *data, const Model::DataInfo &info, size_t &size);
    size_t estimateSize(const Model::DataInfo &info) const;
    void write(uint8_t *&data, const Model::DataInfo &info) const;

private:
    struct PrivateContext;
    PrivateContext *m_context;

    VPVL2_DISABLE_COPY_AND_ASSIGN(Material)
};

} /* namespace pmd2 */
} /* namespace vpvl2 */

#endif
