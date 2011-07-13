/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn                                    */
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

#ifndef VPVL_BONE_H_
#define VPVL_BONE_H_

#include <LinearMath/btAlignedObjectArray.h>
#include <LinearMath/btQuaternion.h>
#include <LinearMath/btTransform.h>
#include <LinearMath/btVector3.h>
#include "vpvl/common.h"

namespace vpvl
{

class Bone;
typedef btAlignedObjectArray<Bone*> BoneList;

/**
 * @file
 * @author Nagoya Institute of Technology Department of Computer Science
 * @author hkrn
 *
 * @section DESCRIPTION
 *
 * Bone class represents a bone of a Polygon Model Data object.
 */

class Bone
{
public:

    /**
     * Type of bone kinds.
     */
    enum Type
    {
        kRotate,
        kRotateAndMove,
        kIKDestination,
        kUnknown,
        kUnderIK,
        kUnderRotate,
        kIKTarget,
        kInvisible,
        kTwist,
        kFollowRotate
    };

    /**
     * Constructor
     */
    Bone();
    ~Bone();

    /**
     * The max length of a bone name
     */
    static const int kNameSize = 20;

    /**
     * Get the center bone name in Shift_JIS, not in english.
     *
     * @return The center bone name in Shift_JIS
     */
    static const uint8_t *centerBoneName();

    /**
     * Get the center bone from a bone list that is not empty.
     *
     * @return The center bone
     */
    static Bone *centerBone(const BoneList *bones);

    /**
     * Stride length of a bone structure.
     *
     * @return Stride length
     */
    static size_t stride();

    /**
     * Read and parse the buffer with size and sets it's result to the class.
     *
     * @param The buffer to read and parse
     * @param Size of the buffer
     */
    void read(const uint8_t *data, BoneList *bones, Bone *rootBone);

    /**
     * Calcurate offset of the bone.
     */
    void computeOffset();

    /**
     * Reset all bone states.
     */
    void reset();

    /**
     * Mark the bone motion independent.
     */
    void setMotionIndependency();

    /**
     * Search a target bone to rotate from all bones.
     *
     * @param All bones with the model
     */
    void setTargetBone(BoneList *bones);

    /**
     * Update rotation from the bone.
     */
    void updateRotation();

    /**
     * Update transform from the bone.
     */
    void updateTransform();

    /**
     * Update rotation from the bone.
     *
     * @param The quaternion value to rotate
     */
    void updateTransform(const btQuaternion &q);

    /**
     * Get transform for skinning
     *
     * @param The result value of transform
     */
    void getSkinTransform(btTransform &tr);

    /**
     * Get the name of bone.
     *
     * @return the name of bone
     */
    const uint8_t *name() const {
        return m_name;
    }

    /**
     * Get the name of bone in English.
     *
     * @return the name of bone in English.
     */
    const uint8_t *englishName() const {
        return m_englishName;
    }

    /**
     * Get the kind of bone.
     *
     * @return the kind of bone
     */
    Type type() const {
        return m_type;
    }

    /**
     * Get the parent bone of this.
     *
     * @return parent bone
     */
    const Bone *parent() const {
        return m_parentBone;
    }

    /**
     * Get the child bone of this.
     *
     * @return child bone
     */
    const Bone *child() const {
        return m_childBone;
    }

    /**
     * Get the target bone of this.
     *
     * @return target bone
     */
    const Bone *target() const {
        return m_targetBone;
    }

    /**
     * Get transform object.
     *
     * @return transform object
     */
    const btTransform &transform() const {
        return m_transform;
    }

    /**
     * Get offset of the bone.
     *
     * @return offset of the bone
     */
    const btVector3 &offset() const {
        return m_offset;
    }

    /**
     * Get original position of the bone.
     *
     * @return original position of the bone
     */
    const btVector3 &originPosition() const {
        return m_originPosition;
    }

    /**
     * Get position of the bone.
     *
     * @return position of the bone
     */
    const btVector3 &position() const {
        return m_position;
    }

    /**
     * Get rotation of the bone.
     *
     * @return rotation of the bone
     */
    const btQuaternion &rotation() const {
        return m_rotation;
    }

    /**
     * Get the bone is constrainted only X coordinate.
     *
     * @return true if the bone is constrainted
     */
    bool isConstraintedXCoordinateForIK() const {
        return m_constraintedXCoordinateForIK;
    }

    /**
     * Get the bone is simulated.
     *
     * @return true if the bone is simulated
     */
    bool isSimulated() const {
        return m_simulated;
    }

    /**
     * Get the bone is independent.
     *
     * @return true if the bone is independent
     */
    bool hasMotionIndependency() const {
        return m_motionIndepent;
    }

    /**
     * Set the name of the bone.
     *
     * @param the name of the bone
     */
    void setName(const uint8_t *value) {
        copyBytesSafe(m_name, value, sizeof(m_name));
    }

    /**
     * Set the name of the bone in English.
     *
     * @param the name of the bone in English
     */
    void setEnglishName(const uint8_t *value) {
        copyBytesSafe(m_englishName, value, sizeof(m_englishName));
    }

    /**
     * Set transform object
     *
     * @param transform object
     */
    void setTransform(const btTransform &value) {
        m_transform = value;
    }

    /**
     * Set offset of the bone.
     *
     * @param offset of the bone
     */
    void setOffset(const btVector3 &value) {
        m_offset = value;
    }

    /**
     * Set position of the bone.
     *
     * @param position of the bone
     */
    void setPosition(const btVector3 &value) {
        m_position = value;
    }

    /**
     * Set rotation of the bone.
     *
     * @param rotation of the bone
     */
    void setRotation(const btQuaternion &value) {
        m_rotation = value;
    }

    /**
     * Set the bone is simulated.
     *
     * @param wheter the bone is simulated
     */
    void setSimulated(bool value) {
        m_simulated = value;
    }

private:
    uint8_t m_name[kNameSize];
    uint8_t m_englishName[kNameSize];
    Type m_type;
    btTransform m_transform;
    btTransform m_transformMoveToOrigin;
    btVector3 m_originPosition;
    btVector3 m_position;
    btVector3 m_offset;
    btQuaternion m_rotation;
    float m_rotateCoef;
    Bone *m_parentBone;
    Bone *m_childBone;
    Bone *m_targetBone;
    int16_t m_targetBoneID;
    bool m_parentIsRoot;
    bool m_constraintedXCoordinateForIK;
    bool m_simulated;
    bool m_motionIndepent;

    VPVL_DISABLE_COPY_AND_ASSIGN(Bone)
};

} /* namespace vpvl */

#endif
