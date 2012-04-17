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
    Bone();
    ~Bone();

    static bool preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info);
    static bool loadBones(const Array<Bone *> &bones, Array<Bone *> &bpsBones, Array<Bone *> &apsBones);

    void read(const uint8_t *data, const Model::DataInfo &info, size_t &size);
    void write(uint8_t *data, const Model::DataInfo &info) const;
    size_t estimateSize(const Model::DataInfo &info) const;
    void mergeMorph(Morph::Bone *morph, float weight);
    void performFullTransform();
    void performTransform();
    void performInverseKinematics();
    void performUpdateLocalTransform();
    void resetIKLink();
    const Vector3 &offset() const;
    const Transform &localTransform() const;

    void setPosition(const Vector3 &value);
    void setRotation(const Quaternion &value);
    void setLocalTransform(const Transform &value);
    void setSimulated(bool value);

    Bone *parentBone() const { return m_parentBone; }
    Bone *targetBone() const { return m_targetBone; }
    Bone *parentInherenceBone() const { return m_parentInherenceBone; }
    Bone *destinationOriginBone() const { return m_destinationOriginBone; }
    const IString *name() const { return m_name; }
    const IString *englishName() const { return m_englishName; }
    const Quaternion &rotation() const { return m_rotation; }
    const Vector3 &origin() const { return m_origin; }
    const Vector3 &destinationOrigin() const { return m_destinationOrigin; }
    const Vector3 &position() const { return m_position; }
    const Vector3 &axis() const { return m_fixedAxis; }
    const Vector3 &axisX() const { return m_axisX; }
    const Vector3 &axisZ() const { return m_axisZ; }
    float constraintAngle() const { return m_angleConstraint; }
    float weight() const { return m_weight; }
    int index() const { return m_index; }
    int layerIndex() const { return m_layerIndex; }
    int externalIndex() const { return m_globalID; }

    bool isRotateable() const { return m_flags & 0x0002; }
    bool isMovable() const { return m_flags & 0x0004; }
    bool isVisible() const { return m_flags & 0x0008; }
    bool isOperatable() const { return m_flags & 0x0010; }
    bool isIKEnabled() const { return m_flags & 0x0020; }
    bool hasPositionInherence() const { return m_flags & 0x0100; }
    bool hasRotationInherence() const { return m_flags & 0x0200; }
    bool isAxisFixed() const { return m_flags & 0x0400; }
    bool hasLocalAxis() const { return m_flags & 0x0800; }
    bool isTransformedAfterPhysicsSimulation() const { return m_flags & 0x1000; }
    bool isTransformedByExternalParent() const { return m_flags & 0x2000; }

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
    void setTransformedAfterPhysicsSimulationEnable(bool value);
    void setTransformedByExternalParentEnable(bool value);

private:
    Array<IKLink *> m_IKLinks;
    Bone *m_parentBone;
    Bone *m_targetBone;
    Bone *m_parentInherenceBone;
    Bone *m_destinationOriginBone;
    IString *m_name;
    IString *m_englishName;
    Quaternion m_rotation;
    Quaternion m_rotationInherence;
    Quaternion m_rotationMorph;
    Quaternion m_rotationIKLink;
    Transform m_localTransform;
    Transform m_localToOrigin;
    Vector3 m_origin;
    Vector3 m_offset;
    Vector3 m_position;
    Vector3 m_positionInherence;
    Vector3 m_positionMorph;
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
    bool m_simulated;

    VPVL2_DISABLE_COPY_AND_ASSIGN(Bone)
};

} /* namespace pmx */
} /* namespace vpvl2 */

#endif

