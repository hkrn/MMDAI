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

#ifndef VPVL2_PMX_BONE_H_
#define VPVL2_PMX_BONE_H_

#include "vpvl2/IBone.h"
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
 * Bone class represents a bone of a Polygon Model Extended object.
 */

class VPVL2_API Bone : public IBone
{
public:
    struct IKLink;

    /**
     * Constructor
     */
    Bone(IModel *modelRef);
    ~Bone();

    static bool preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info);
    static bool loadBones(const Array<Bone *> &bones, Array<Bone *> &bpsBones, Array<Bone *> &apsBones);
    static size_t estimateTotalSize(const Array<Bone *> &bones, const Model::DataInfo &info);

    void read(const uint8_t *data, const Model::DataInfo &info, size_t &size);
    void write(uint8_t *data, const Model::DataInfo &info) const;
    size_t estimateSize(const Model::DataInfo &info) const;
    void mergeMorph(const Morph::Bone *morph, const IMorph::WeightPrecision &weight);
    void getLocalTransform(Transform &output) const;
    void getLocalTransform(const Transform &worldTransform, Transform &output) const;
    void performFullTransform();
    void performTransform();
    void solveInverseKinematics();
    void performUpdateLocalTransform();
    void resetIKLink();
    Vector3 offset() const { return m_offset; }
    Transform worldTransform() const { return m_worldTransform; }
    Transform localTransform() const { return m_localTransform; }
    void getEffectorBones(Array<IBone *> &value) const;

    void setLocalPosition(const Vector3 &value);
    void setLocalRotation(const Quaternion &value);
    Vector3 fixedAxis() const;
    void getLocalAxes(Matrix3x3 &value) const;
    void setLocalTransform(const Transform &value);
    void setSimulated(bool value);

    IModel *parentModelRef() const { return m_modelRef; }
    IBone *parentBoneRef() const { return m_parentBoneRef; }
    IBone *targetBoneRef() const { return m_targetBoneRef; }
    IBone *parentInherenceBoneRef() const { return m_parentInherenceBoneRef; }
    IBone *destinationOriginBoneRef() const { return m_destinationOriginBoneRef; }
    const IString *name() const { return m_name; }
    const IString *englishName() const { return m_englishName; }
    Quaternion localRotation() const { return m_rotation; }
    Vector3 origin() const { return m_origin; }
    Vector3 destinationOrigin() const;
    Vector3 localPosition() const { return m_localPosition; }
    Vector3 axis() const { return m_fixedAxis; }
    Vector3 axisX() const { return m_axisX; }
    Vector3 axisZ() const { return m_axisZ; }
    float constraintAngle() const { return m_angleConstraint; }
    float weight() const { return m_weight; }
    int index() const { return m_index; }
    int layerIndex() const { return m_layerIndex; }
    int externalIndex() const { return m_globalID; }

    bool isRotateable() const;
    bool isMovable() const;
    bool isVisible() const;
    bool isInteractive() const;
    bool hasInverseKinematics() const;
    bool hasRotationInherence() const;
    bool hasPositionInherence() const;
    bool hasFixedAxes() const;
    bool hasLocalAxes() const;
    bool isTransformedAfterPhysicsSimulation() const;
    bool isTransformedByExternalParent() const;

    void setParentBone(Bone *value);
    void setParentInherenceBone(Bone *value, float weight);
    void setTargetBone(Bone *target, int nloop, float angleConstraint);
    void setDestinationOriginBone(Bone *value);
    void setName(const IString *value);
    void setEnglishName(const IString *value);
    void setOrigin(const Vector3 &value);
    void setDestinationOrigin(const Vector3 &value);
    void setFixedAxis(const Vector3 &value);
    void setAxisX(const Vector3 &value);
    void setAxisZ(const Vector3 &value);
    void setIndex(int value);
    void setLayerIndex(int value);
    void setExternalIndex(int value);
    void setRotateable(bool value);
    void setMovable(bool value);
    void setVisible(bool value);
    void setOperatable(bool value);
    void setIKEnable(bool value);
    void setPositionInherenceEnable(bool value);
    void setRotationInherenceEnable(bool value);
    void setAxisFixedEnable(bool value);
    void setLocalAxisEnable(bool value);
    void setTransformAfterPhysicsEnable(bool value);
    void setTransformedByExternalParentEnable(bool value);
    void setInverseKinematicsEnable(bool value);

private:
    IModel *m_modelRef;
    Array<IKLink *> m_IKLinks;
    Bone *m_parentBoneRef;
    Bone *m_targetBoneRef;
    Bone *m_parentInherenceBoneRef;
    IBone *m_destinationOriginBoneRef;
    IString *m_name;
    IString *m_englishName;
    Quaternion m_rotation;
    Quaternion m_rotationInherence;
    Quaternion m_rotationMorph;
    Quaternion m_rotationIKLink;
    Transform m_worldTransform;
    Transform m_localTransform;
    Vector3 m_origin;
    Vector3 m_offset;
    Vector3 m_localPosition;
    Vector3 m_localPositionInherence;
    Vector3 m_localPositionMorph;
    Vector3 m_destinationOrigin;
    Vector3 m_fixedAxis;
    Vector3 m_axisX;
    Vector3 m_axisZ;
    float m_angleConstraint;
    float m_weight;
    int m_index;
    int m_parentBoneIndex;
    int m_layerIndex;
    int m_destinationOriginBoneIndex;
    int m_targetBoneIndex;
    int m_nloop;
    int m_parentInherenceBoneIndex;
    int m_globalID;
    uint16_t m_flags;
    bool m_enableInverseKinematics;

    VPVL2_DISABLE_COPY_AND_ASSIGN(Bone)
};

} /* namespace pmx */
} /* namespace vpvl2 */

#endif

