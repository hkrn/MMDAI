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

class VPVL2_API Bone
{
public:
    struct IKLink;

    /**
     * Constructor
     */
    Bone();
    ~Bone();

    static bool preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info);
    static bool loadBones(const Array<Bone *> &bones, Array<Bone *> &ordered);

    /**
     * Read and parse the buffer with id and sets it's result to the class.
     *
     * @param data The buffer to read and parse
     */
    void read(const uint8_t *data, const Model::DataInfo &info, size_t &size);
    void write(uint8_t *data) const;
    void mergeMorph(Morph::Bone *morph, float weight);
    void performTransform();
    void performInverseKinematics();
    const Vector3 &offset() const;
    const Transform localTransform() const;

    Bone *parentBone() const { return m_parentBone; }
    Bone *offsetBone() const { return m_offsetBone; }
    Bone *targetBone() const { return m_targetBone; }
    Bone *parentBiasBone() const { return m_parentBiasBone; }
    const StaticString *name() const { return m_name; }
    const StaticString *englishName() const { return m_englishName; }
    const Vector3 &origin() const { return m_origin; }
    const Vector3 &axis() const { return m_fixedAxis; }
    const Vector3 &axisX() const { return m_axisX; }
    const Vector3 &axisZ() const { return m_axisZ; }
    float constraintAngle() const { return m_constraintAngle; }
    float bias() const { return m_bias; }
    float priority() const { return m_priority; }
    int id() const { return m_id; }

    bool isRotateable() const { return m_flags & 0x0002; }
    bool isMovable() const { return m_flags & 0x0004; }
    bool isVisible() const { return m_flags & 0x0008; }
    bool isOperatable() const { return m_flags & 0x0010; }
    bool hasIKLinks() const { return m_flags & 0x0020; }
    bool hasPositionBias() const { return m_flags & 0x0100; }
    bool hasRotationBias() const { return m_flags & 0x0200; }
    bool isAxisFixed() const { return m_flags & 0x0400; }
    bool hasLocalAxis() const { return m_flags & 0x0800; }
    bool isTransformedAfterPhysicsSimulation() const { return m_flags & 0x1000; }
    bool isTransformedByExternalParent() const { return m_flags & 0x2000; }

private:
    Array<IKLink *> m_IKLinks;
    Bone *m_parentBone;
    Bone *m_offsetBone;
    Bone *m_targetBone;
    Bone *m_parentBiasBone;
    StaticString *m_name;
    StaticString *m_englishName;
    Quaternion m_rotation;
    Quaternion m_rotationExtra;
    Quaternion m_rotationMorph;
    Quaternion m_rotationIKLink;
    Transform m_localTransform;
    Transform m_IKLinkTransform;
    Vector3 m_origin;
    Vector3 m_position;
    Vector3 m_positionExtra;
    Vector3 m_positionMorph;
    Vector3 m_offset;
    Vector3 m_fixedAxis;
    Vector3 m_axisX;
    Vector3 m_axisZ;
    float m_constraintAngle;
    float m_bias;
    int m_id;
    int m_parentBoneIndex;
    int m_priority;
    int m_offsetBoneIndex;
    int m_targetBoneIndex;
    int m_nloop;
    int m_nlinks;
    int m_parentBoneBiasIndex;
    int m_globalID;
    uint16_t m_flags;

    VPVL2_DISABLE_COPY_AND_ASSIGN(Bone)
};

} /* namespace pmx */
} /* namespace vpvl2 */

#endif

