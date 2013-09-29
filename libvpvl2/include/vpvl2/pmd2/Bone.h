/**

 Copyright (c) 2010-2013  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef VPVL2_PMD2_BONE_H_
#define VPVL2_PMD2_BONE_H_

#include "vpvl2/Common.h"
#include "vpvl2/IBone.h"
#include "vpvl2/pmd2/Model.h"

namespace vpvl2
{

class IEncoding;
class IString;

namespace pmd2
{

class VPVL2_API Bone VPVL2_DECL_FINAL : public IBone
{
public:
    enum Type {
        kRotate,
        kRotateAndMove,
        kIKRoot,
        kUnknown,
        kIKJoint,
        kUnderRotate,
        kIKEffector,
        kInvisible,
        kTwist,
        kFollowRotate,
        kMaxType
    };

    static const int kNameSize;
    static const int kCategoryNameSize;

    Bone(Model *parentModelRef, IEncoding *encodingRef);
    ~Bone();

    void addEventListenerRef(PropertyEventListener *value);
    void removeEventListenerRef(PropertyEventListener *value);
    void getEventListenerRefs(Array<PropertyEventListener *> &value);

    const IString *name(IEncoding::LanguageType type) const;
    void setName(const IString *value, IEncoding::LanguageType type);
    int index() const;
    IModel *parentModelRef() const;
    IBone *parentBoneRef() const;
    IBone *effectorBoneRef() const;
    Transform worldTransform() const;
    Transform localTransform() const;
    void getLocalTransform(Transform &world2LocalTransform) const;
    void getLocalTransform(const Transform &worldTransform, Transform &output) const;
    void setLocalTransform(const Transform &value);
    Vector3 origin() const;
    Vector3 destinationOrigin() const;
    Vector3 localTranslation() const;
    Quaternion localOrientation() const;
    void getEffectorBones(Array<IBone *> &value) const;
    void setLocalTranslation(const Vector3 &value);
    void setLocalOrientation(const Quaternion &value);
    void setTargetBoneRef(IBone *value);
    bool isMovable() const;
    bool isRotateable() const;
    bool isVisible() const;
    bool isInteractive() const;
    bool hasInverseKinematics() const;
    bool hasFixedAxes() const;
    bool hasLocalAxes() const;
    Vector3 fixedAxis() const;
    void getLocalAxes(Matrix3x3 &value) const;
    void setInverseKinematicsEnable(bool value);
    void setIndex(int value);

    static bool preparseBones(uint8 *&ptr, vsize &rest, Model::DataInfo &info);
    static bool loadBones(const Array<Bone *> &bones);
    static void writeBones(const Array<Bone *> &bones, const Model::DataInfo &info, uint8 *&data);
    static void writeEnglishNames(const Array<Bone *> &morphs, const Model::DataInfo &info, uint8 *&data);
    static vsize estimateTotalSize(const Array<Bone *> &bones, const Model::DataInfo &info);

    void readBone(const uint8 *data, const Model::DataInfo &info, vsize &size);
    void readEnglishName(const uint8 *data, int index);
    void write(uint8 *&data, const Model::DataInfo &info) const;
    void performTransform();
    void setSimulated(bool value);
    bool isAxisXAligned();
    bool isInverseKinematicsEnabled() const;
    float32 coefficient() const;

    void setParentBoneRef(IBone * /* value */) {}
    void setParentInherentBoneRef(IBone * /* value */, float32 /* weight */) {}
    void setEffectorBoneRef(IBone * /* effector */, int /* numIteration */, float /* angleLimit */) {}
    void setDestinationOriginBoneRef(IBone * /* value */) {}
    void setOrigin(const Vector3 & /* value */) {}
    void setDestinationOrigin(const Vector3 & /* value */) {}
    void setFixedAxis(const Vector3 & /* value */) {}
    void setAxisX(const Vector3 & /* value */) {}
    void setAxisZ(const Vector3 & /* value */) {}
    void setLayerIndex(int /* value */) {}
    void setExternalIndex(int /* value */) {}
    void setRotateable(bool /* value */) {}
    void setMovable(bool /* value */) {}
    void setVisible(bool /* value */) {}
    void setInteractive(bool /* value */) {}
    void setHasInverseKinematics(bool /* value */) {}
    void setInherentOrientationEnable(bool /* value */) {}
    void setInherentTranslationEnable(bool /* value */) {}
    void setAxisFixedEnable(bool /* value */) {}
    void setLocalAxesEnable(bool /* value */) {}
    void setTransformAfterPhysicsEnable(bool /* value */) {}
    void setTransformedByExternalParentEnable(bool /* value */) {}

private:
    struct PrivateContext;
    PrivateContext *m_context;

    VPVL2_DISABLE_COPY_AND_ASSIGN(Bone)
};

} /* namespace pmd2 */
} /* namespace vpvl2 */

#endif
