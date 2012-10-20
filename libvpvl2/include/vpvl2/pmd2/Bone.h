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

class VPVL2_API Bone : public IBone
{
public:
    enum Type {
        kRotate,
        kRotateAndMove,
        kIKDestination,
        kUnknown,
        kUnderIK,
        kUnderRotate,
        kIKTarget,
        kInvisible,
        kTwist,
        kFollowRotate,
        kMaxType
    };

    static const int kNameSize = 20;
    static const int kCategoryNameSize = 50;

    Bone(IEncoding *encodingRef);
    ~Bone();

    const IString *name() const;
    int index() const;
    IBone *parentBone() const;
    IBone *targetBone() const;
    const Transform &worldTransform() const;
    const Transform &localTransform() const;
    void getLocalTransform(Transform &world2LocalTransform) const;
    void getLocalTransform(const Transform &worldTransform, Transform &output) const;
    void setLocalTransform(const Transform &value);
    const Vector3 &origin() const;
    const Vector3 destinationOrigin() const;
    const Vector3 &localPosition() const;
    const Quaternion &rotation() const;
    void getEffectorBones(Array<IBone *> &value) const;
    void setLocalPosition(const Vector3 &value);
    void setRotation(const Quaternion &value);
    bool isMovable() const;
    bool isRotateable() const;
    bool isVisible() const;
    bool isInteractive() const;
    bool hasInverseKinematics() const;
    bool hasFixedAxes() const;
    bool hasLocalAxes() const;
    const Vector3 &fixedAxis() const;
    void getLocalAxes(Matrix3x3 &value) const;
    void setInverseKinematicsEnable(bool value);

    static bool preparseBones(uint8_t *&ptr, size_t &rest, Model::DataInfo &info);
    static bool preparseIKConstraints(uint8_t *&ptr, size_t &rest, Model::DataInfo &info);
    static bool loadBones(const Array<Bone *> &bones);
    static void readIKConstraint(const uint8_t *data, const Array<Bone *> &bones, size_t &size);
    static size_t estimateTotalSize(const Array<Bone *> &bones, const Model::DataInfo &info);

    void readBone(const uint8_t *data, const Model::DataInfo &info, size_t &size);
    size_t estimateBoneSize(const Model::DataInfo &info) const;
    size_t estimateIKConstraintsSize(const Model::DataInfo &info) const;
    void write(uint8_t *data, const Model::DataInfo &info) const;
    void performTransform();
    void solveInverseKinematics();
    void setSimulated(bool value);
    bool isAxisXAligned();

private:
    struct IKConstraint;
    IEncoding *m_encodingRef;
    IString *m_name;
    IBone *m_parentBoneRef;
    IBone *m_targetBoneRef;
    IBone *m_childBoneRef;
    IKConstraint *m_constraint;
    Vector3 m_fixedAxis;
    Vector3 m_origin;
    Vector3 m_offset;
    Vector3 m_localPosition;
    Quaternion m_rotation;
    Transform m_worldTransform;
    Transform m_localTransform;
    Type m_type;
    int m_index;
    int m_parentBoneIndex;
    int m_targetBoneIndex;
    int m_childBoneIndex;
    bool m_enableInverseKinematics;
};

} /* namespace pmd2 */
} /* namespace vpvl2 */

#endif
