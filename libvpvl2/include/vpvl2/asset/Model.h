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

#ifndef VPVL2_ASSET_MODEL_H_
#define VPVL2_ASSET_MODEL_H_

#include "vpvl2/Common.h"
#include "vpvl2/IBone.h"
#include "vpvl2/IEncoding.h"
#include "vpvl2/IModel.h"
#include "vpvl2/IMorph.h"
#include "vpvl2/IString.h"

#include "vpvl/Asset.h"

class btDiscreteDynamicsWorld;

namespace vpvl2
{
namespace asset
{

class VPVL2_API Model : public IModel
{
public:
    Model(IEncoding *encoding);
    ~Model();

    Type type() const { return kAsset; }
    const IString *name() const { return m_name; }
    const IString *englishName() const { return m_name; }
    const IString *comment() const { return m_name; }
    const IString *englishComment() const { return m_name; }
    bool isVisible() const { return true; }
    Error error() const { return kNoError; }
    bool load(const uint8_t *data, size_t size);
    void save(uint8_t * /* data */) const {}
    size_t estimateSize() const { return 1; }
    void resetVertices() {}
    void performUpdate() {}
    void joinWorld(btDiscreteDynamicsWorld * /* world */) {}
    void leaveWorld(btDiscreteDynamicsWorld * /* world */) {}
    IBone *findBone(const IString * /* value */) const { return 0; }
    IMorph *findMorph(const IString * /* value */) const { return 0; }
    int count(Object /* value */) const { return 0; }
    void getBones(Array<IBone *> & /* value */) const {}
    void getMorphs(Array<IMorph *> & /* value */) const {}
    const Vector3 &position() const { return m_asset.position(); }
    const Quaternion &rotation() const { return m_asset.rotation(); }
    const Scalar &opacity() const { return m_asset.opacity(); }
    const Scalar &scaleFactor() const { return m_asset.scaleFactor(); }
    IModel *parentModel() const { return m_parentModel; }
    IBone *parentBone() const { return m_parentBone; }
    void setName(const IString *value);
    void setEnglishName(const IString *value) { setName(value); }
    void setComment(const IString *value);
    void setEnglishComment(const IString *value) { setComment(value); }
    void setPosition(const Vector3 &value) { m_asset.setPosition(value); }
    void setRotation(const Quaternion &value) { m_asset.setRotation(value); }
    void setOpacity(const Scalar &value) { m_asset.setOpacity(value); }
    void setScaleFactor(const Scalar &value) { m_asset.setScaleFactor(value); }
    void setParentModel(IModel *value) { m_parentModel = value; }
    void setParentBone(IBone *value) { m_parentBone = value; }

    vpvl::Asset *ptr() { return &m_asset; }

private:
    vpvl::Asset m_asset;
    IEncoding *m_encoding;
    IString *m_name;
    IString *m_comment;
    Array<IBone *> m_bones;
    Array<IMorph *> m_morphs;
    IModel *m_parentModel;
    IBone *m_parentBone;
};

} /* namespace asset */
} /* namespace vpvl2 */

#endif
